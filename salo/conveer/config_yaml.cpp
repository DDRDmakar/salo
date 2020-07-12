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

#include "headers/config_yaml.h"
#include "headers/muxamed.h"
#include "headers/parse_tools.h"
#include "headers/exception.h"

namespace Convspace
{

// Constructors
Config_yaml::Config_yaml(const std::string &plain_text) // Constructor from string
{
	logs -> CommitConveer( F, L, "Object initialization - Config_yaml" );
	put_data(plain_text);
}
Config_yaml::Config_yaml(const YAML::Node &parent_node) // Constructor from node
{
	logs -> CommitConveer( F, L, "Object initialization - Config_yaml" );
	config_storage.reset();
	config_storage = parent_node;
	// put_data(show());
}

// Destructor
Config_yaml::~Config_yaml(void)
{
	// logs->CommitConveer(F, L, "Object destruction - Config_yaml");
	config_storage.reset();
}

// Recursively read, if there is such data
const YAML::Node Config_yaml::unsafe_subtree(const std::string &target)
{
	if (target.empty()) return config_storage;
	
	const size_t last_slash = target.find_last_of("/");
	
	const std::string afterslash = last_slash == std::string::npos ? target : target.substr(last_slash + 1, target.size());
	
	unsigned int afterslash_index = 0;
	bool is_list = false;
	if (std::regex_match(thin_to_wide_line(afterslash), std::wregex(L"^\\[\\d+\\]$")))
	{
		is_list = true;
		afterslash_index = std::stoi(afterslash.substr(1, afterslash.length() - 2));
	}
	
	// If slash exists and it is not last slash in the string
	if (last_slash == std::string::npos)
	{
		// No more slashes, path is first part
		if (is_list)
		{
			if (afterslash_index < config_storage.size()) return config_storage[afterslash_index];
			else throw std::invalid_argument("Node " + target + " does not exist");
		}
		else
		{
			if (config_storage[afterslash]) return config_storage[afterslash];
			else throw std::invalid_argument("Node " + afterslash + " does not exist");
		}
	}
	else
	{
		// Text from last slash to the end of string is key of this structure level
		if (last_slash + 1 == target.size()) throw std::invalid_argument("Invalid address syntax (slash in the end): " + target);
		if (afterslash.empty()) throw std::invalid_argument("Invalid address syntax (double slash): " + target);
		
		// Just another CONST node
		const YAML::Node howtonameit = (is_list) ?
			(unsafe_subtree(target.substr(0, last_slash))[afterslash_index]) : (unsafe_subtree(target.substr(0, last_slash))[afterslash]);
		if (howtonameit) return howtonameit;
		else throw std::invalid_argument("Node " + target + " does not exist");
	}
}
const YAML::Node Config_yaml::subtree(const std::string &target)
{
	try { return unsafe_subtree(target); }
	
	catch ( YAML::KeyNotFound &e )
	{
		error_out( "ERROR reading config - object \"" + target + "\" not found in config. " + e.what() );
		return YAML::Node();
	}
	catch ( YAML::InvalidNode &e )
	{
		error_out( "ERROR reading config - object \"" + target + "\" not found in config. " + e.what() );
		return YAML::Node();
	}
	catch ( std::invalid_argument &e )
	{
		error_out( "ERROR reading config - address \"" + target + "\" has incorrect syntax. " + e.what() );
		return YAML::Node();
	}
	catch ( YAML::Exception &e )
	{
		error_out( "ERROR reading config - address \"" + target + "\" has incorrect syntax. " + e.what() );
		return YAML::Node();
	}
}

YAML::Node Config_yaml::unsafe_tree(void)
{
	return subtree();
}
YAML::Node Config_yaml::editable_subtree(const std::string &target)
{
	return subtree(target);
}

void Config_yaml::put_data( const std::string &plain_text )
{
	try
	{
		// Parse file
		config_storage.reset();
		if (!plain_text.empty()) config_storage = YAML::Load( plain_text );
	}
	catch ( YAML::ParserException &e ) { error_out( std::string( "ERROR - config parsing terminated: " + std::string( e.what() ) ) ); }
}

// _______________________________________________________________________________________________________________________________________________
// _________|  P U B L I C  |______________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________

// YAML emitter
std::string Config_yaml::show(const std::string &path)
{
	YAML::Emitter out;
	out << subtree(path);
	return out.c_str();
}

// +==============================[ L O G I C A L ]==============================+

bool Config_yaml::there_is_element(const std::string &target)
{
	try
	{
		const size_t last_slash = target.find_last_of("/");
		const std::string afterslash = last_slash == std::string::npos ? target : target.substr(last_slash + 1, target.size());
		unsigned int afterslash_index = 0;
		bool is_list = false;
		if (std::regex_match(thin_to_wide_line(afterslash), std::wregex(L"^\\[\\d+\\]$")))
		{
			is_list = true;
			afterslash_index = std::stoi(afterslash.substr(1, afterslash.length() - 2));
		}
		
		if (last_slash + 1 == target.length()) return false;
		
		const std::string path_to_element = (last_slash == std::string::npos) ? "" : target.substr(0, last_slash);
		
		return is_list ? (bool)unsafe_subtree(path_to_element)[afterslash_index] : (bool)unsafe_subtree(path_to_element)[afterslash];
	}
	catch (std::exception) { return false; }
}
/*
Config_yaml Config_yaml::copy(const std::string &path)
{
	return Config_yaml(show(path));
}
*/
// +==============================[ P R I M A R Y ]==============================+

int Config_yaml::get_coefficient( const std::string &target )
{
	try { return subtree( target ).as <int> (); }
	catch ( YAML::Exception &e )
	{
		error_out( "ERROR reading config - coefficient \"" + target + "\" has incomplete format in config. " + e.what() );
		return 0;
	}
}

std::string Config_yaml::get_keyword( const std::string &target )
{
	try { return subtree( target ).as <std::string> (); }
	catch ( YAML::Exception &e )
	{
		error_out( "ERROR reading config - keyword \"" + target + "\" has incomplete format in config. " + e.what() );
		return "";
	}
}

std::vector <std::string> Config_yaml::get_group( const std::string &target )
{
	try
	{
		YAML::Node current_node = subtree( target );
		std::vector <std::string> res;
		for( YAML::const_iterator it = current_node.begin(); it != current_node.end(); ++it )
		{
			res.push_back( it -> as <std::string> () );
		}
		return res;
	}
	catch ( YAML::Exception &e )
	{
		error_out( "ERROR reading config - group \"" + target + "\" has incomplete format in config. " + e.what() );
		return std::vector <std::string> ();
	}
}

std::string Config_yaml::get_group( const std::string &target, const unsigned int position )
{
	try
	{
		YAML::Node config_value = subtree( target );
		if ( config_value.size() > position ) return config_value[ position ].as <std::string> ();
		else throw std::invalid_argument( "Index is out of range" );
	}
	catch ( std::invalid_argument &e )
	{
		error_out(  "ERROR reading config - reading element from group \"" + target + "\" failed in config. " + e.what() );
		return "";
	}
	catch ( YAML::Exception &e )
	{
		error_out(  "ERROR reading config - reading element from group \"" + target + "\" failed in config. " + e.what() );
		return "";
	}
}

int Config_yaml::get_group_size( const std::string &target )
{
	try { return subtree( target ).size(); }
	catch ( YAML::BadConversion &e )
	{
		error_out(  "ERROR reading config - size of group \"" + target + "\" failed in config. " + e.what() );
		return 0;
	}
}

bool Config_yaml::get_binary( const std::string &target )
{
	try { return subtree( target ).as <bool> (); }
	catch ( YAML::BadConversion &e )
	{
		error_out( "ERROR reading config - boolean \"" + target + "\" has incomplete format in config. " + e.what() );
		return false;
	}
}

bool Config_yaml::get_possibility( const std::string &target )
{
	try
	{
		const YAML::Node current_tf_storage = subtree( target );
		int possibility_true  = current_tf_storage["t"].as <int> ();
		int possibility_false = current_tf_storage["f"].as <int> ();
		if (possibility_true + possibility_false == 0) return false;
		return rand() % ( possibility_true + possibility_false ) < possibility_true;
	}
	catch ( YAML::KeyNotFound &e )
	{
		error_out( "ERROR reading config - possibility structure \"" + target + "\" has no members \"t\" and \"f\" in config. " + e.what() );
		return false;
	}
	catch ( YAML::Exception &e )
	{
		error_out( "ERROR reading config - possibility structure members \"" + target + "\" have incomplete format in config. " + e.what() );
		return false;
	}
}

// +==============================[ S E C O N D A R Y ]==============================+

std::string Config_yaml::get_group_random( const std::string &target )
{
	int current_group_size = get_group_size( target );
	return current_group_size == 0 ? "" : get_group( target, rand() % current_group_size );
}

bool Config_yaml::one_matches( const std::string &target, const std::string &elem )
{
	std::vector <std::string> current_group = get_group( target );
	for ( auto e : current_group ) if ( e == elem ) return true;
	return false;
}

std::regex Config_yaml::get_regex( const std::string &target )
{
	return std::regex( get_keyword( target ) );
}

// +==============================[ W I D E S T R I N G ]==============================+

bool Config_yaml::there_is_element(const std::wstring &target) { return there_is_element(wide_to_thin_line(target)); }

int Config_yaml::get_coefficient( const std::wstring &target ) { return get_coefficient( wide_to_thin_line( target ) ); }
std::wstring Config_yaml::get_keyword( const std::wstring &target ) { return thin_to_wide_line( get_keyword( wide_to_thin_line( target ) ) ); }
std::vector <std::wstring> Config_yaml::get_group( const std::wstring &target ) { return thin_to_wide_storage( get_group( wide_to_thin_line( target ) ) ); }
std::wstring Config_yaml::get_group( const std::wstring &target, const unsigned int position ) { return thin_to_wide_line( get_group( wide_to_thin_line( target ), position ) ); }
int Config_yaml::get_group_size( const std::wstring &target ) { return get_group_size( wide_to_thin_line( target ) ); }
bool Config_yaml::get_binary( const std::wstring &target ) { return get_binary( wide_to_thin_line( target ) ); }
bool Config_yaml::get_possibility( const std::wstring &target ) { return get_possibility( wide_to_thin_line( target ) ); }

std::wstring Config_yaml::get_group_random( const std::wstring &target ) { return thin_to_wide_line( get_group_random( wide_to_thin_line( target ) ) ); }
bool Config_yaml::one_matches( const std::wstring &target, const std::wstring &elem ) { return one_matches( wide_to_thin_line( target ), wide_to_thin_line( elem ) ); }
std::wregex Config_yaml::get_regex( const std::wstring &target ) { return std::wregex( get_keyword( target ) ); }



// +==============================[ E D I T I N G ]==============================+

void Config_yaml::insert_into_set(const std::string &path, const std::string &value)
{
	if (there_is_element(path)) editable_subtree(path).push_back(value);
	else throw Convspace::Exception("Error inserting value \"" + value + "\" into sequence \"" + path + "\" - node does not exist.");
}

void Config_yaml::set_value(const std::string &path, const std::string &value)
{
	if (there_is_element(path)) editable_subtree(path) = value;
	else throw Convspace::Exception("Error setting value \"" + value + "\" into node \"" + path + "\" - node does not exist.");
}

void Config_yaml::clear_node(const std::string &path)
{
	// Works only with map elements
	// FOr sequences use functions below
	const int slash_index = path.find_last_of("/");
	if (slash_index + 1 == path.length()) throw Convspace::Exception("Error clearing node \"" + path + "\" - slash in the end of path.");
	const std::string path_to_node_to_clear = (slash_index == std::string::npos) ? "" : path.substr(0, slash_index);
	const std::string name_of_node_to_clear = (slash_index == std::string::npos) ? path : path.substr(slash_index + 1, path.length());
	try
	{
		YAML::Node current_node = unsafe_subtree(path_to_node_to_clear);
		current_node.remove(name_of_node_to_clear);
	}
	catch (YAML::Exception e) { throw Convspace::Exception("Error clearing value \"" + name_of_node_to_clear + "\" into node \"" + path_to_node_to_clear + "\": " + std::string(e.what())); }
}

void Config_yaml::seq_delete_index(const std::string &path, const int index)
{
	try
	{
		if (there_is_element(path)) editable_subtree(path).remove(index);
		else throw std::exception();
	}
	catch (std::exception) { throw Convspace::Exception("Error deleting element from sequence \"" + path + "\" by index \"" + std::to_string(index) + "\"."); }
}

void Config_yaml::seq_delete_value(const std::string &path, const std::string &value)
{
	try
	{
		YAML::Node current_seq = unsafe_subtree(path);
		for (int i = 0; i < current_seq.size(); ++i)
			if (current_seq[i].as<std::string>() == value) current_seq.remove(i);
	}
	catch (YAML::Exception) { throw Convspace::Exception("Error deleting element from sequence \"" + path + "\" by value \"" + value + "\"."); }
}

void Config_yaml::create_node(const std::string &path)
{
	// Creates empty list
	try
	{
		if (there_is_element(path)) return;
		
		const int slash_index = path.find_last_of("/");
		if (slash_index + 1 == path.length()) throw Convspace::Exception("Error creating node \"" + path + "\" - slash in the end of path.");
		const std::string path_to_node = (slash_index == std::string::npos) ? "" : path.substr(0, slash_index);
		const std::string name_of_node = (slash_index == std::string::npos) ? path : path.substr(slash_index + 1, path.length());
		if (!path_to_node.empty() && !there_is_element(path_to_node)) create_node(path_to_node);
		editable_subtree(path_to_node)[name_of_node].push_back("a");
		editable_subtree(path_to_node)[name_of_node].remove(0);
	}
	catch (YAML::Exception e) { throw Convspace::Exception("Error creating node \"" + path + "\": " + std::string(e.what())); }
}

void Config_yaml::add_to_string_num(const std::string &path, const int v)
{
	if (there_is_element(path))
	{
		try { editable_subtree(path) = std::to_string(get_coefficient(path) + v); }
		catch (std::exception) { throw Convspace::Exception("Error incrementing string value \"" + path + "\" - incorrect number format."); }
	}
	else throw Convspace::Exception("Error incrementing string value \"" + path + "\" - node does not exist.");
}

} // namespace Convspace
