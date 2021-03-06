#include <iostream>
#include <string>
#include "../include/imageProcess.h"

int main(int argc, char *argv[]){
    if (argc < 3)
        std::cerr << "Usage: " << argv[0] << " <operation> <inputfile1> [inputfile2]" << std::endl;
    else{
        std::string operation(argv[1]);
        std::string input_file(argv[2]);
        std::string other_file;
        if (argc > 3)
            other_file = std::string(argv[3]);

        std::string output_file = "output/output.png";

        if (operation == "grayscale")
            convert_grayscale(input_file, output_file);
        else if (operation == "blur")
            gaussian_blur(input_file, output_file);
        else if (operation == "tonemap")
            tonemap_HDR(input_file, output_file);
        else if (operation == "redeye")
            if (argc > 3)
                remove_redeye(input_file, other_file, output_file);
            else
                std::cerr << "redeye requires a second input template file" << std::endl;
        else if (operation == "clone")
            if (argc > 3)
                seamless_clone(input_file, other_file, output_file);
            else
                std::cerr << "clone requires a second input source file" << std::endl;
        else
            std::cerr << "Unknown operation - options: grayscale, blur, tonemap, redeye, clone" << std::endl;
    }
}
