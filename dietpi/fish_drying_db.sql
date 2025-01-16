CREATE DATABASE sensor_data;

-- Step 1: Switch to the sensor_data database
USE sensor_data;

-- Step 2: Create the drying_session table
CREATE TABLE drying_session (
    id INT AUTO_INCREMENT PRIMARY KEY,         -- Unique ID for each session
    type VARCHAR(255) NOT NULL,                -- Type as a string
    trays_used INT NOT NULL DEFAULT 0,         -- Number of trays used
    start_timestamp TIMESTAMP NOT NULL,        -- Start time of the session
    weight_start FLOAT NOT NULL,               -- Starting weight
    humidity_start FLOAT NOT NULL,             -- Starting humidity
    end_timestamp TIMESTAMP DEFAULT NULL,      -- End time of the session (nullable for ongoing sessions)
    weight_end FLOAT DEFAULT NULL,             -- Ending weight (nullable for ongoing sessions)
    humidity_end FLOAT DEFAULT NULL            -- Ending humidity (nullable for ongoing sessions)
);

-- Step 1: Create the post_drying_session table
CREATE TABLE post_drying_session (
    id INT PRIMARY KEY,                        -- Same ID as in drying_session
    assessment_date TIMESTAMP NOT NULL,        -- Calculated from drying_session's end_timestamp + 7 days
    time_duration INT DEFAULT NULL,            -- Duration in minutes
    weight_loss FLOAT DEFAULT NULL,            -- Weight loss during drying
    humidity_loss FLOAT DEFAULT NULL;          -- Humidity loss during drying
    mold_growth BOOLEAN DEFAULT FALSE,         -- Boolean for mold growth assessment
    pinking_reddening BOOLEAN DEFAULT NULL,    -- Boolean for pinking/reddening
    souring BOOLEAN DEFAULT NULL,              -- Boolean for souring
    case_hardening BOOLEAN DEFAULT NULL,       -- Boolean for case hardening
    rancidity BOOLEAN DEFAULT NULL,            -- Boolean for rancidity
    dun_pepperyspots BOOLEAN DEFAULT NULL,     -- Boolean for dun peppery spots
    stains BOOLEAN DEFAULT NULL,               -- Boolean for stains
    tough_texture BOOLEAN DEFAULT NULL,        -- Boolean for tough texture

    -- Define the foreign key relationship
    CONSTRAINT fk_drying_session FOREIGN KEY (id)
    REFERENCES drying_session(id)
    ON DELETE CASCADE                           -- Delete post_drying_session if drying_session is deleted
);

DELIMITER $$

CREATE TRIGGER after_drying_session_insert
AFTER UPDATE ON drying_session
FOR EACH ROW
BEGIN
    -- Only populate post_drying_session if end_timestamp is not NULL
    IF NEW.end_timestamp IS NOT NULL THEN
        INSERT INTO post_drying_session (
            id, assessment_date, time_duration, weight_loss, humidity_loss,
            mold_growth, pinking_reddening, souring, case_hardening, rancidity,
            dun_pepperyspots, stains, tough_texture, remarks
        )
        VALUES (
            NEW.id,
            DATE_ADD(NEW.end_timestamp, INTERVAL 7 DAY), -- Assessment date
            TIMESTAMPDIFF(MINUTE, NEW.start_timestamp, NEW.end_timestamp), -- Time duration in minutes
            NEW.weight_start - NEW.weight_end,           -- Weight loss
            NEW.humidity_start - NEW.humidity_end,       -- Humidity loss
            FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, NULL
        );
    END IF;
END$$

DELIMITER ;
