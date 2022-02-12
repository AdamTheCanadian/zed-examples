
/********************************************************************************
 ** This sample demonstrates how to log high rate imu data and images at the same time                                                
 ********************************************************************************/

// Standard includes
#include <stdio.h>
#include <string.h>

// ZED include
#include <sl/Camera.hpp>


// Sample functions
void updateCameraSettings(char key, sl::Camera &zed);
void switchCameraSettings();
void printHelp();
void print(std::string msg_prefix, sl::ERROR_CODE err_code = sl::ERROR_CODE::SUCCESS, std::string msg_suffix = "");
void parseArgs(int argc, char **argv,sl::InitParameters& param);

// Sample variables
sl::VIDEO_SETTINGS camera_settings_ = sl::VIDEO_SETTINGS::BRIGHTNESS;
std::string str_camera_settings = "BRIGHTNESS";
int step_camera_setting = 1;
bool led_on = true;

bool selectInProgress = false;
sl::Rect selection_rect;

int main(int argc, char **argv) {
    // Create a ZED Camera object

    std::cout << " Reboot CAMERA " << std::endl;
    sl::ERROR_CODE errcode= sl::Camera::reboot(0);
    sl::sleep_ms(3000);
    std::cout << " Reboot CAMERA --> " << errcode << std::endl;
    sl::Camera zed;
    
    sl::InitParameters init_parameters;
    init_parameters.sdk_verbose = true;
    init_parameters.camera_resolution= sl::RESOLUTION::HD720;
    init_parameters.depth_mode = sl::DEPTH_MODE::NONE; // no depth computation required here
    parseArgs(argc,argv, init_parameters);

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
    
    // Print help in console
    printHelp();

    // Initialise camera setting
    switchCameraSettings();

    sl::Mat zed_image;
    // Capture new images until 'q' is pressed
    char key = ' ';
    while (key != 'q') {
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
        
        // Change camera settings with keyboard
        updateCameraSettings(key, zed);        
    }

    // Exit
    zed.close();
    return EXIT_SUCCESS;
}

/**
    This function updates camera settings
 **/
void updateCameraSettings(char key, sl::Camera &zed) {
    int current_value;

    // Keyboard shortcuts
    switch (key) {

        // Switch to the next camera parameter
        case 's':
        switchCameraSettings();
        current_value = zed.getCameraSettings(camera_settings_);
        break;

        // Increase camera settings value ('+' key)
        case '+':
		current_value = zed.getCameraSettings(camera_settings_);
		zed.setCameraSettings(camera_settings_, current_value + step_camera_setting);
        print(str_camera_settings+": " + std::to_string(zed.getCameraSettings(camera_settings_)));
        break;

        // Decrease camera settings value ('-' key)
        case '-':
		current_value = zed.getCameraSettings(camera_settings_);
        current_value = current_value > 0 ? current_value - step_camera_setting : 0; // take care of the 'default' value parameter:  VIDEO_SETTINGS_VALUE_AUTO
        zed.setCameraSettings(camera_settings_, current_value);
        print(str_camera_settings+": " + std::to_string(zed.getCameraSettings(camera_settings_)));
        break;

        //switch LED On :
        case 'l':
        led_on = !led_on;
        zed.setCameraSettings(sl::VIDEO_SETTINGS::LED_STATUS, led_on);
        break;

        // Reset to default parameters
        case 'r':
        print("Reset all settings to default\n");
        for(int s = (int)sl::VIDEO_SETTINGS::BRIGHTNESS; s <= (int)sl::VIDEO_SETTINGS::WHITEBALANCE_TEMPERATURE; s++)
            zed.setCameraSettings(static_cast<sl::VIDEO_SETTINGS>(s), sl::VIDEO_SETTINGS_VALUE_AUTO);
        break;

        case 'a':
        {
            std::cout<<"[Sample] set AEC_AGC_ROI on target ["<<selection_rect.x<<","<<selection_rect.y<<","<<selection_rect.width<<","<<selection_rect.height<<"]\n";
        zed.setCameraSettings(sl::VIDEO_SETTINGS::AEC_AGC_ROI, selection_rect, sl::SIDE::BOTH);
        }
        break;

        case 'f':
        print("reset AEC_AGC_ROI to full res");
        zed.setCameraSettings(sl::VIDEO_SETTINGS::AEC_AGC_ROI,selection_rect,sl::SIDE::BOTH,true);
        break;

    }
}

/**
    This function toggles between camera settings
 **/
void switchCameraSettings() {
    camera_settings_ = static_cast<sl::VIDEO_SETTINGS>((int)camera_settings_ + 1);

    // reset to 1st setting
    if (camera_settings_ == sl::VIDEO_SETTINGS::LED_STATUS)
        camera_settings_ = sl::VIDEO_SETTINGS::BRIGHTNESS;

    // increment if AEC_AGC_ROI since it using the overloaded function
    if (camera_settings_ == sl::VIDEO_SETTINGS::AEC_AGC_ROI)
        camera_settings_ = static_cast<sl::VIDEO_SETTINGS>((int)camera_settings_ + 1);
    
    // select the right step
    step_camera_setting = (camera_settings_ == sl::VIDEO_SETTINGS::WHITEBALANCE_TEMPERATURE) ? 100 : 1;

    // get the name of the selected SETTING
    str_camera_settings = std::string(sl::toString(camera_settings_).c_str());
    
    print("Switch to camera settings: ", sl::ERROR_CODE::SUCCESS, str_camera_settings);
}

/**
    This function displays help
 **/
void printHelp() {
    std::cout << "\n\nCamera controls hotkeys:\n";
    std::cout << "* Increase camera settings value:  '+'\n";
    std::cout << "* Decrease camera settings value:  '-'\n";
    std::cout << "* Toggle camera settings:          's'\n";
    std::cout << "* Toggle camera LED:               'l' (lower L)\n";
    std::cout << "* Reset all parameters:            'r'\n";
    std::cout << "* Reset exposure ROI to full image 'f'\n";
    std::cout << "* Use mouse to select an image area to apply exposure (press 'a')\n";
    std::cout << "* Exit :                           'q'\n\n";
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

void parseArgs(int argc, char **argv,sl::InitParameters& param)
{
    if (argc > 1 && std::string(argv[1]).find(".svo")!= std::string::npos) {
        // SVO input mode not available in camera control
        std::cout<<"SVO Input mode is not available for camera control sample"<< std::endl;
    } else if (argc > 1 && std::string(argv[1]).find(".svo")== std::string::npos) {
        std::string arg = std::string(argv[1]);
        unsigned int a,b,c,d,port;
        if (sscanf(arg.c_str(),"%u.%u.%u.%u:%d", &a, &b, &c, &d,&port) == 5) {
            // Stream input mode - IP + port
            std::string ip_adress = std::to_string(a)+"."+ std::to_string(b)+"."+ std::to_string(c)+"."+ std::to_string(d);
            param.input.setFromStream(sl::String(ip_adress.c_str()),port);
            std::cout<<"[Sample] Using Stream input, IP : "<<ip_adress<<", port : "<<port<< std::endl;
        }
        else  if (sscanf(arg.c_str(),"%u.%u.%u.%u", &a, &b, &c, &d) == 4) {
            // Stream input mode - IP only
            param.input.setFromStream(sl::String(argv[1]));
            std::cout<<"[Sample] Using Stream input, IP : "<<argv[1]<< std::endl;
        }
        else if (arg.find("HD2K")!= std::string::npos) {
            param.camera_resolution = sl::RESOLUTION::HD2K;
            std::cout<<"[Sample] Using Camera in resolution HD2K"<< std::endl;
        } else if (arg.find("HD1080")!= std::string::npos) {
            param.camera_resolution = sl::RESOLUTION::HD1080;
            std::cout<<"[Sample] Using Camera in resolution HD1080"<< std::endl;
        } else if (arg.find("HD720")!= std::string::npos) {
            param.camera_resolution = sl::RESOLUTION::HD720;
            std::cout<<"[Sample] Using Camera in resolution HD720"<< std::endl;
        } else if (arg.find("VGA")!= std::string::npos) {
            param.camera_resolution = sl::RESOLUTION::VGA;
            std::cout<<"[Sample] Using Camera in resolution VGA"<< std::endl;
        }
    } else {
        // Default
    }
}
