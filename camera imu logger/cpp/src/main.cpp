
/********************************************************************************
 ** This sample demonstrates how to log high rate imu data and images at the same time                                                
 ********************************************************************************/

// Standard includes
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <ctime>
#include <thread>
#include <fstream>

// ZED include
#include <sl/Camera.hpp>

static bool exit_app = false;
// File name for where imu data will be logged to, since logging is done on a separate thread
// we make this global
static std::string imu_file_name;

// Create a ZED Camera object
static sl::Camera zed;

// Handle the CTRL-C keyboard signal
void CtrlHandler(DWORD fdwCtrlType) {
    exit_app = (fdwCtrlType == CTRL_C_EVENT);
}

// Set the function to handle the CTRL-C
void SetCtrlHandler() {
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
}

void print(std::string msg_prefix, 
           sl::ERROR_CODE err_code = sl::ERROR_CODE::SUCCESS, 
           std::string msg_suffix = "");

// Basic structure to compare timestamps of a sensor. Determines if a specific sensor data has been updated or not.
struct TimestampHandler {

    // Compare the new timestamp to the last valid one. If it is higher, save it as new reference.
    inline bool isNew(sl::Timestamp& ts_curr, sl::Timestamp& ts_ref) {
        bool new_ = ts_curr > ts_ref;
        if (new_) ts_ref = ts_curr;
        return new_;
    }
    // Specific function for IMUData.
    inline bool isNew(sl::SensorsData::IMUData& imu_data) {
        return isNew(imu_data.timestamp, ts_imu);
    }
    // Specific function for MagnetometerData.
    inline bool isNew(sl::SensorsData::MagnetometerData& mag_data) {
        return isNew(mag_data.timestamp, ts_mag);
    }
    // Specific function for BarometerData.
    inline bool isNew(sl::SensorsData::BarometerData& baro_data) {
        return isNew(baro_data.timestamp, ts_baro);
    }

    sl::Timestamp ts_imu = 0, ts_baro = 0, ts_mag = 0; // Initial values
};


void logImuData() {
    std::ofstream imu_file(imu_file_name, std::ios::out);
    if (!imu_file.is_open()) {
        std::cout << "Could not open imu file \n\t" << imu_file_name << std::endl;
        return;
    }
    std::cout << "Opened imu file " << imu_file_name << std::endl;

    // Used to store sensors timestamps and check if new data is available.
    TimestampHandler ts;

    // Used to store sensors data.
    sl::SensorsData sensors_data;

    while (!exit_app) {
        if (zed.getSensorsData(sensors_data, sl::TIME_REFERENCE::CURRENT) == sl::ERROR_CODE::SUCCESS) {
            if (ts.isNew(sensors_data.imu)) {
                imu_file << sensors_data.imu.timestamp.getMicroseconds() << ","
                    << sensors_data.imu.angular_velocity[0] << ","
                    << sensors_data.imu.angular_velocity[1] << ","
                    << sensors_data.imu.angular_velocity[2] << ","
                    << sensors_data.imu.linear_acceleration[0] << ","
                    << sensors_data.imu.linear_acceleration[1] << ","
                    << sensors_data.imu.linear_acceleration[2] << "\n";
            }
        }
    }

    imu_file.close();
}

int main(int argc, char **argv) {
    
    sl::InitParameters init_parameters;
    init_parameters.camera_resolution= sl::RESOLUTION::HD2K;
    // We are just logging data, so we dont need to compute depth
    init_parameters.depth_mode = sl::DEPTH_MODE::NONE;

    // Open the camera
    auto returned_state = zed.open(init_parameters);
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        print("Camera Open", returned_state, "Exit program.");
        return EXIT_FAILURE;
    }

    // Setup output svo file name
    time_t rawtime;
    struct tm* time_info;
    char buffer[256];
    time(&rawtime);
    time_info = localtime(&rawtime);

    std::strftime(buffer, sizeof(buffer), "C:\\Users\\aclare\\Documents\\%Y_%m_%d_%H_%M_%S.svo", time_info);
    std::string svo_file_name(buffer);
    std::strftime(buffer, sizeof(buffer), "C:\\Users\\aclare\\Documents\\%Y_%m_%d_%H_%M_%S.imu", time_info);
    imu_file_name = std::string(buffer);

    std::cout << "SVO: " << svo_file_name << std::endl;
    std::cout << "IMU: " << imu_file_name << std::endl;

    sl::String svo_name = sl::String(svo_file_name.c_str());
    returned_state = zed.enableRecording(
        sl::RecordingParameters(svo_name, sl::SVO_COMPRESSION_MODE::H264));
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        print("Recording ZED : ", returned_state);
        zed.close();
        return EXIT_FAILURE;
    }

    std::thread imu_thread(logImuData);

    SetCtrlHandler();
    while (!exit_app) {
        // Check that a new image is successfully acquired
        returned_state = zed.grab();
    }

    // Exit
    zed.disableRecording();
    imu_thread.join();
    zed.close();
    return EXIT_SUCCESS;
}

void print(std::string msg_prefix, sl::ERROR_CODE err_code, std::string msg_suffix) {
    std::cout <<"[Sample]";
    if (err_code != sl::ERROR_CODE::SUCCESS)
        std::cout << "[Error] ";
    else
        std::cout<<" ";
    std::cout << msg_prefix << " ";
    if (err_code != sl::ERROR_CODE::SUCCESS) {
        std::cout << " | " << toString(err_code) << " : ";
        std::cout << toVerbose(err_code);
    }
    if (!msg_suffix.empty())
        std::cout << " " << msg_suffix;
    std::cout << std::endl;
}
