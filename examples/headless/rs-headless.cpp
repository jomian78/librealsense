// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015-2017 Intel Corporation. All Rights Reserved.

#include <librealsense/rs2.hpp> // Include RealSense Cross Platform API

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams

// 3rd party header for writing png files
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Helper function for writing metadata to disk as a csv file
void metadata_to_csv(const rs2::frame& frm, const std::string& filename);

// This sample captures 30 frames and writes the last frame to disk.
// It can be useful for debugging an embedded system with no display.
int main(int argc, char * argv[]) try
{
    using namespace rs2;

    // Declare depth colorizer for pretty visualization of depth data
    colorizer color_map;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    pipeline pipe;
    // Start streaming with default recommended configuration
    pipe.start();

    // Capture 30 frames to give autoexposure, etc. a chance to settle
    for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();
    
    // Wait for the next set of frames from the camera. Now that autoexposure, etc.
    // has settled, we will write these to disk
    for (auto&& frame : pipe.wait_for_frames())
    {
        // We can only save video frames as pngs, so we skip the rest
        if (auto vf = frame.as<video_frame>())
        {
            auto stream = frame.get_profile().stream_type();
            // Use the colorizer to get an rgb image for the depth stream
            if (stream == RS2_STREAM_DEPTH) vf = color_map(frame);

            // Write images to disk
            std::stringstream png_file;
            png_file << "rs-headless-output-" << vf.get_profile().stream_name() << ".png";
            stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(),
                           vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());

            // Record per-frame metadata for UVC streams
            std::stringstream csv_file;
            csv_file << "rs-headless-output-" << vf.get_profile().stream_name()
                     << "-metadata.csv";
            metadata_to_csv(vf, csv_file.str());
        }
    }

    return EXIT_SUCCESS;
}
catch(const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
}
catch(const std::exception & e)
{
    std::cerr << e.what() << std::endl;
}

void metadata_to_csv(const rs2::frame& frm, const std::string& filename)
{
    std::ofstream csv;

    csv.open(filename);

    //    std::cout << "Writing metadata to " << filename << endl;
    csv << "Stream," << rs2_stream_to_string(frm.get_profile().stream_type()) << "\nMetadata Attribute,Value\n";

    // Record all the available metadata attributes
    for (size_t i = 0; i < RS2_FRAME_METADATA_COUNT; i++)
    {
        if (frm.supports_frame_metadata((rs2_frame_metadata)i))
        {
            csv << rs2_frame_metadata_to_string((rs2_frame_metadata)i) << ","
                << frm.get_frame_metadata((rs2_frame_metadata)i) << "\n";
        }
    }

    csv.close();
}
