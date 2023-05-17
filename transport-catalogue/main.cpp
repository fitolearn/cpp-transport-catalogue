#include <iostream>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"
int main()
{
	transport_catalogue::TransportCatalogue t_c;
	transport_catalogue::InputReader i_r(t_c);
	i_r.ReadRequests(std::cin);
	transport_catalogue::StatReader s_r(t_c);
	s_r.OutputRequests(std::cin, std::cerr);
	return 0;
}
