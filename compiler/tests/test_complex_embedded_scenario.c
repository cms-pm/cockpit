/*
 * Complex Embedded Scenario Test
 * Real-world embedded system simulation combining all language features
 * Simulates a sensor monitoring system with state machine and data processing
 */

// System state variables
int system_state;
int sensor_readings[8];
int reading_index;
int error_count;
int total_readings;
int alert_threshold;

// Processing variables  
int running_average;
int min_reading;
int max_reading;
int processing_flags;

// System states
// 0 = INIT, 1 = MONITORING, 2 = PROCESSING, 3 = ALERT, 4 = ERROR

void init_system() {
    system_state = 0;
    reading_index = 0;
    error_count = 0;
    total_readings = 0;
    alert_threshold = 80;
    running_average = 0;
    min_reading = 1000;
    max_reading = 0;
    processing_flags = 0;
    
    // Initialize sensor array
    int i = 0;
    while (i < 8) {
        sensor_readings[i] = 0;
        i = i + 1;
    }
}

int read_sensor_simulation(int sensor_id) {
    // Simulate sensor reading based on ID and system state
    int base_value = 50 + (sensor_id * 10);
    int variation = (total_readings % 7) - 3;  // -3 to +3 variation
    return base_value + variation;
}

void store_sensor_reading(int value) {
    if (reading_index >= 8) {
        reading_index = 0;  // Circular buffer
    }
    
    sensor_readings[reading_index] = value;
    reading_index = reading_index + 1;
    total_readings = total_readings + 1;
}

int calculate_statistics() {
    int sum = 0;
    int count = 0;
    min_reading = 1000;
    max_reading = 0;
    
    int i = 0;
    while (i < 8) {
        if (sensor_readings[i] > 0) {  // Valid reading
            sum = sum + sensor_readings[i];
            count = count + 1;
            
            if (sensor_readings[i] < min_reading) {
                min_reading = sensor_readings[i];
            }
            if (sensor_readings[i] > max_reading) {
                max_reading = sensor_readings[i];
            }
        }
        i = i + 1;
    }
    
    if (count > 0) {
        running_average = sum / count;
        return 1;  // Success
    } else {
        return 0;  // No valid readings
    }
}

int check_alert_conditions() {
    int alert_flags = 0;
    
    // Check threshold
    if (running_average > alert_threshold) {
        alert_flags = alert_flags | 1;  // High average alert
    }
    
    // Check range
    if ((max_reading - min_reading) > 40) {
        alert_flags = alert_flags | 2;  // High variation alert
    }
    
    // Check error rate
    if (error_count > (total_readings / 10)) {
        alert_flags = alert_flags | 4;  // High error rate alert
    }
    
    return alert_flags;
}

void process_data_pipeline() {
    processing_flags = 0;
    
    // Multi-stage data processing
    if (calculate_statistics()) {
        processing_flags = processing_flags | 1;  // Stats calculated
        
        // Data validation stage
        if ((running_average > 20) && (running_average < 120)) {
            processing_flags = processing_flags | 2;  // Valid range
            
            // Trend analysis simulation
            int trend_direction = 0;
            if (sensor_readings[reading_index - 1] > running_average) {
                trend_direction = 1;  // Increasing
            } else if (sensor_readings[reading_index - 1] < running_average) {
                trend_direction = -1; // Decreasing
            }
            
            if (trend_direction != 0) {
                processing_flags = processing_flags | 4;  // Trend detected
            }
        }
    }
}

void system_state_machine() {
    int next_state = system_state;
    
    if (system_state == 0) {  // INIT
        if (total_readings >= 3) {
            next_state = 1;  // -> MONITORING
        }
    } else if (system_state == 1) {  // MONITORING
        if (total_readings % 5 == 0) {
            next_state = 2;  // -> PROCESSING
        }
        if (error_count > 5) {
            next_state = 4;  // -> ERROR
        }
    } else if (system_state == 2) {  // PROCESSING
        process_data_pipeline();
        
        int alert_flags = check_alert_conditions();
        if (alert_flags > 0) {
            next_state = 3;  // -> ALERT
        } else {
            next_state = 1;  // -> MONITORING
        }
    } else if (system_state == 3) {  // ALERT
        // Simulate alert handling
        if ((processing_flags & 7) == 7) {  // All processing flags set
            next_state = 1;  // -> MONITORING (alert cleared)
        }
    } else if (system_state == 4) {  // ERROR
        if (error_count < 3) {
            next_state = 0;  // -> INIT (reset)
        }
    }
    
    system_state = next_state;
}

void setup() {
    // Initialize the embedded system
    init_system();
    
    // Simulate system operation over multiple cycles
    int cycle = 0;
    while (cycle < 15) {
        // Simulate sensor reading
        int sensor_value = read_sensor_simulation(cycle % 4);
        
        // Validate reading (simulate occasional errors)
        if ((sensor_value > 20) && (sensor_value < 120)) {
            store_sensor_reading(sensor_value);
        } else {
            error_count = error_count + 1;
        }
        
        // Run state machine
        system_state_machine();
        
        // System health check
        if ((system_state == 4) && (cycle > 10)) {
            error_count = 1;  // Simulate error recovery
        }
        
        cycle = cycle + 1;
    }
    
    // Final system status
    int final_alert_status = check_alert_conditions();
    
    printf("Embedded scenario: state=%d, readings=%d, avg=%d, alerts=%d, flags=%d\n",
           system_state, total_readings, running_average, final_alert_status, processing_flags);
}