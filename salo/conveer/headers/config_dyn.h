
/*
*
*  Copyright ©, 2015-2020. All Rights Reserved.
*
*    Autors:
*    Motylenok Mikhail
*    Makarevich Nikita
*
*    This code is privately owned and is a commercial secret. We do not provide
*    code to anyone without the written agreement. Copying, publication, use
*    for commercial or non-commercial purposes without the consent
*    of the authors is a violation of applicable law.
*
*/


#ifndef __SEQUENCE_CHECK_CONFIG_DYN__
#define __SEQUENCE_CHECK_CONFIG_DYN__

#include "config_yaml.h"
#include "config.h"

using namespace std;

/*
 * 
 * Dynamic bot parameters, which should be stored permanently
 * 
 */

namespace Convspace
{

	class Config_dyn : public Config_yaml
	{
		
	private:
		
		Config *config;
		
		std::string path;
		
	public:
		
		// Constructor
		Config_dyn(Config *transit_config = NULL);
		
		void read_from_file(void);
		void write_to_file(void);
		void synchronize(void);
		
		// Public open node editing
		// They throw Convspace::Exception(), be careful!
		void dyn_insert_into_set(const std::string &path, const std::string &value);  // Insert element into sequence
		void dyn_set_value(const std::string &path, const std::string &value);        // Override existing value ( map support only! )
		void dyn_clear_node(const std::string &path = "");                            // Clear node
		void dyn_seq_delete_index(const std::string &path, const int index);          // Delete element from sequence by index
		void dyn_seq_delete_value(const std::string &path, const std::string &value); // Delete element from sequence by value
		void dyn_create_node(const std::string &path);                                // Create empty node (default node = {} empty map)
		void dyn_add_to_string_num(const std::string &path, const int v);             // Increment number value, which is stored as string
	};

} // namespace Convspace

#endif
