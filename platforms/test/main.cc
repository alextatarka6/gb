#include "../../src/gameboy_prelude.h"
#include "../cli/cli.h"

static std::unique_ptr<CartridgeInfo> info;

int main(int argc, char* argv[]) {
    CliOptions cliOptions = get_cli_options(argc, argv);
    auto rom_data = read_bytes(cliOptions.filename);
    info = get_info(rom_data);
    return 0;
}