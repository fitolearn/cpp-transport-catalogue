#pragma once

#include "transport_catalogue.h"

namespace transport {

    void FillCatalogue(Catalogue& catalogue);

    namespace detail {

        Bus FillRoute(std::string& line);
        Stop FillStop(std::string& line);
        void FillStopDistances(std::string& line, Catalogue& catalogue);

    } // namespace detail

} // namespace transport
