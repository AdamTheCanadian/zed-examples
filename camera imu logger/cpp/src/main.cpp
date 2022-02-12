
/********************************************************************************
 ** This sample demonstrates how to log high rate imu data and images at the same time                                                
 ********************************************************************************/

// Standard includes
#include <stdio.h>
#include <string.h>
#include <Windows.h>

// ZED include
#include <sl/Camera.hpp>


static bool exit_app = false;

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

// Sample variables
sl::VIDEO_SETTINGS camera_settings_ = sl::VIDEO_SETTINGS::BRIGHTNESS;
std::string str_camera_settings = "BRIGHTNESS";

int main(int argc, char **argv) {
    // Create a ZED Camera object
    sl::Camera zed;
    
    sl::InitParameters init_parameters;
    init_parameters.sdk_verbose = true;
    init_parameters.camera_resolution= sl::RESOLUTION::HD720;
    // We are just logging data, so we dont need to compute depth
    init_parameters.depth_mode = sl::DEPTH_MODE::NONE;

    // Open the camera
    auto returned_state = zed.open(init_parameters);
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        print("Camera Open", returned_state, "Exit program.");
        return EXIT_FAILURE;
    }

    // Print camera information
    auto camera_info = zed.getCameraInformation();
    std::cout << std::endl;
    std::cout <<"ZED Model                 : "<<camera_info.camera_model << std::endl;
    std::cout <<"ZED Serial Number         : "<< camera_info.serial_number << std::endl;
    std::cout <<"ZED Camera Firmware       : "<< camera_info.camera_configuration.firmware_version <<"/"<<camera_info.sensors_configuration.firmware_version<< std::endl;
    std::cout <<"ZED Camera Resolution     : "<< camera_info.camera_configuration.resolution.width << "x" << camera_info.camera_configuration.resolution.height << std::endl;
    std::cout <<"ZED Camera FPS            : "<< zed.getInitParameters().camera_fps << std::endl;
    
    sl::Mat zed_image;
    
    SetCtrlHandler();
    char key = ' ';
    while (!exit_app) {
        // Check that a new image is successfully acquired
        returned_state = zed.grab();
        if (returned_state == sl::ERROR_CODE::SUCCESS) {
            // Retrieve left image
            zed.retrieveImage(zed_image, sl::VIEW::LEFT);

        }
        else {
            print("Error during capture : ", returned_state);
            break;
        }  
    }

    // Exit
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
