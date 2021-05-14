#include "ast.hpp"
#include "macros.hpp"

#include <cstdlib>

#include <absl/status/status.h>
#include <absl/status/statusor.h>

namespace rdss {

absl::Status RealMain() {
    return absl::OkStatus();
}

}  // namespace rdss

int main() {
    absl::Status result = rdss::RealMain();
    if(!result.ok()) {
        std::cerr << result << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
