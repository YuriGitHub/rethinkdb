
#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <pthread.h>
#include "protocol.hpp"

using namespace std;

/* Information shared between clients */
struct shared_t {
public:
    typedef protocol_t*(*protocol_factory_t)(config_t*);
    
public:
    shared_t(config_t *_config, protocol_factory_t _protocol_factory)
        : config(_config),
          qps_offset(0), latencies_offset(0),
          qps_fd(NULL), latencies_fd(NULL),
          protocol_factory(_protocol_factory),
          last_qps(0), n_op(1), n_tick(1), n_ops_so_far(0)
        {
            pthread_mutex_init(&mutex, NULL);
            
            if(config->qps_file[0] != 0) {
                qps_fd = fopen(config->qps_file, "wa");
            }
            if(config->latency_file[0] != 0) {
                latencies_fd = fopen(config->latency_file, "wa");
            }
        }

    ~shared_t() {
        if(qps_fd) {
            fwrite(qps, 1, qps_offset, qps_fd);
            fclose(qps_fd);
        }
        if(latencies_fd) {
            fwrite(latencies, 1, latencies_offset, latencies_fd);
            fclose(latencies_fd);
        }

        pthread_mutex_destroy(&mutex);
    }
    
    void push_qps(int _qps, int tick) {
        if(!qps_fd && !latencies_fd)
            return;
        
        lock();

        // Collect qps info from all clients before attempting to print
        int qps_count = 0, agg_qps = 0;
        map<int, pair<int, int> >::iterator op = qps_map.find(tick);
        if(op != qps_map.end()) {
            qps_count = op->second.first;
            agg_qps = op->second.second;
        }

        qps_count += 1;
        agg_qps += _qps;
        
        if(qps_count == config->clients) {
            _qps = agg_qps;
            qps_map.erase(op);
        } else {
            qps_map[tick] = pair<int, int>(qps_count, agg_qps);
            unlock();
            return;
        }
            
        last_qps = _qps;

        if(!qps_fd) {
            unlock();
            return;
        }
        
        int _off = snprintf(qps + qps_offset, sizeof(qps) - qps_offset, "%d\t\t%d\n", n_tick, _qps);
        if(_off >= sizeof(qps) - qps_offset) {
            // Couldn't write everything, flush
            fwrite(qps, 1, qps_offset, qps_fd);
            
            // Write again
            qps_offset = 0;
            _off = snprintf(qps + qps_offset, sizeof(qps) - qps_offset, "%d\t\t%d\n", n_tick, _qps);
        }
        qps_offset += _off;

        n_tick++;
        
        unlock();
    }

    void push_latency(float latency) {
        n_ops_so_far++;
        /*
        if(n_ops_so_far % 200000 == 0)
            printf("%ld\n", n_ops_so_far);
        */
        if(!latencies_fd || last_qps == 0)
            return;

        // We cannot possibly write every latency because that stalls
        // the client, so we want to scale that by the number of qps
        // (we'll sample latencies for roughly N random ops every
        // second).
        const int samples_per_second = 20;
        if(rand() % (last_qps / samples_per_second) != 0) {
            return;
        }

        lock();
        
        int _off = snprintf(latencies + latencies_offset, sizeof(latencies) - latencies_offset, "%ld\t\t%.2f\n", n_op, latency);
        if(_off >= sizeof(latencies) - latencies_offset) {
            // Couldn't write everything, flush
            fwrite(latencies, 1, latencies_offset, latencies_fd);
            
            // Write again
            latencies_offset = 0;
            _off = snprintf(latencies + latencies_offset, sizeof(latencies) - latencies_offset, "%ld\t\t%.2f\n", n_op, latency);
        }
        latencies_offset += _off;

        n_op++;

        unlock();
    }

public:
    protocol_factory_t protocol_factory;
    
private:
    config_t *config;
    map<int, pair<int, int> > qps_map;
    char qps[40960], latencies[40960];
    int qps_offset, latencies_offset;
    FILE *qps_fd, *latencies_fd;
    pthread_mutex_t mutex;
    int last_qps;

    long n_op;
    int n_tick;
    long n_ops_so_far;

private:
    void lock() {
        pthread_mutex_lock(&mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex);
    }
};

/* Communication structure for main thread and clients */
struct client_data_t {
    config_t *config;
    shared_t *shared;
};

/* The function that does the work */
void* run_client(void* data) {
    // Grab the config
    client_data_t *client_data = (client_data_t*)data;
    config_t *config = client_data->config;
    shared_t *shared = client_data->shared;
    shared_t::protocol_factory_t pf = shared->protocol_factory;
    protocol_t *proto = (*pf)(config);
    
    // Connect to the server
    proto->connect(config);

    // Store the keys so we can run updates and deletes.
    vector<payload_t> keys;
    vector<payload_t> op_keys;

    // Perform the ops
    ticks_t last_time = get_ticks(), last_qps_time = last_time, now_time;
    int qps = 0, tick = 0;
    int total_queries = 0;
    while(total_queries < config->duration / config->clients) {
        // Generate the command
        load_t::load_op_t cmd = config->load.toss((config->batch_factor.min + config->batch_factor.max) / 2.0f);

        int _val;
        payload_t key, value;
        int j, k, l; // because we can't declare in the loop
        
        switch(cmd) {
        case load_t::delete_op:
            // Find the key
            if(keys.empty())
                break;
            _val = random(0, keys.size() - 1);
            key = keys[_val];
            // Delete it from the server
            proto->remove(key.first, key.second);
            // Our own bookkeeping
            free(key.first);
            keys[_val] = keys[keys.size() - 1];
            keys.erase(keys.begin() + _val);
            qps++;
            total_queries++;
            break;
            
        case load_t::update_op:
            // Find the key and generate the payload
            if(keys.empty())
                break;
            key = keys[random(0, keys.size() - 1)];
            config->values.toss(&value);
            // Send it to server
            proto->update(key.first, key.second, value.first, value.second);
            // Free the value
            free(value.first);
            qps++;
            total_queries++;
            break;
            
        case load_t::insert_op:
            // Generate the payload
            config->keys.toss(&key);
            config->values.toss(&value);
            // Send it to server
            proto->insert(key.first, key.second, value.first, value.second);
            // Free the value and save the key
            free(value.first);
            keys.push_back(key);
            qps++;
            total_queries++;
            break;
            
        case load_t::read_op:
            // Find the key
            if(keys.empty())
                break;
            op_keys.clear();
            j = random(config->batch_factor.min, config->batch_factor.max);
            j = std::min(j, (int)keys.size());
            l = random(0, keys.size() - 1);
            for(k = 0; k < j; k++) {
                key = keys[l];
                l++;
                if(l >= keys.size())
                    l = 0;
                op_keys.push_back(key);
            }
            // Read it from the server
            proto->read(&op_keys[0], j);
            qps += j;
            total_queries += j;
            break;
        };
        now_time = get_ticks();

        // Deal with individual op latency
        ticks_t latency = now_time - last_time;
        shared->push_latency(ticks_to_us(latency));
        last_time = now_time;

        // Deal with QPS
        if(ticks_to_secs(now_time - last_qps_time) >= 1.0f) {
            shared->push_qps(qps, tick);

            last_qps_time = now_time;
            qps = 0;
            tick++;
        }
    }

    delete proto;
    
    // Free all the keys
    for(vector<payload_t>::iterator i = keys.begin(); i != keys.end(); i++) {
        free(i->first);
    }
}

#endif // __CLIENT_HPP__

