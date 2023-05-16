#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"
#include "geo.h"
#include <iostream>

int main()
{
	transport_catalogue::TransportCatalogue t_c;
	transport_catalogue::InputReader i_r(t_c);
	i_r.ReadRequests(std::cin);
	transport_catalogue::StatReader s_r(t_c);
	s_r.OutputRequests(std::cin);
    int i;
    cin >> i;
	return 0;
}
