#include "orion-shader-compiler/compiler.h"

#include <CLI/CLI.hpp>

#include <fstream>
#include <iostream>
#include <string>

int main(int argc, const char* argv[])
{
    auto cli_app = CLI::App{"Command line interface for the Orion Shader Compiler", "orion-shader-compiler"};

    std::string output;
    cli_app.add_option("-o,--output", output, "Output filename");

    std::string entry_point;
    cli_app.add_option("-e,--entry", entry_point, "Entry point name")
        ->required();

    std::string target;
    cli_app.add_option("-t,--target", target, "Target profile")
        ->required();

    bool spirv = false;
    bool dxil = false;
    {
        auto* ir_group = cli_app.add_option_group("IL Target", "Intermediate language target");
        ir_group->require_option(1);
        ir_group->add_flag("--spirv", spirv);
        ir_group->add_flag("--dxil", dxil);
    }

    bool debug = false;
    cli_app.add_flag("-d,--debug", debug, "Enable debug information");

    std::string input;
    cli_app.add_option("input", input, "Input file")
        ->required()
        ->check(CLI::ExistingFile);

    CLI11_PARSE(cli_app, argc, argv)

    const auto compile_desc = orion::ShaderCompileDesc{
        .filename = input,
        .entry_point = entry_point,
        .target_profile = target,
        .shader_il = spirv ? orion::ShaderIL::SpirV : orion::ShaderIL::DXIL,
        .debug = debug,
    };
    auto compile_result = orion::shader_compiler::compile_file(input, compile_desc);

    if (!compile_result) {
        return static_cast<int>(compile_result.error().error);
    }

    auto write_output = [&compile_success = compile_result.value()](std::ostream& outstream) {
        for (const char byte : compile_success.object) {
            outstream << byte;
        }
        return outstream.good();
    };

    bool output_good = false;
    if (output.empty()) {
        output_good = write_output(std::cout);
    } else {
        auto outfile = std::ofstream{output, std::ios::out | std::ios::binary | std::ios::trunc};
        output_good = write_output(outfile);
    }

    if (!output_good) {
        std::cerr << "Failed to create_write output\n";
        return 1;
    }

    return 0;
}
