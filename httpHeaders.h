//HTTPHeaders.h

#include <vector>
#include <string>
#include <list>

#define cr '\r'
#define lf '\n'

typedef struct requestParameter {
	std::string name;
	std::list<std::string> values;
} requestParameter;

enum method {M_GET, M_HEAD, M_OPTIONS, M_TRACE, M_POST, M_PUT, M_DELETE, M_VERSION};

class requestHeader {
	private:
		
	public:
		method op;
		std::string resource;
		std::string version;
		std::vector<requestParameter> parameters;
		bool keepAlive;
		bool host;
		
		requestHeader ();
		void refresh ();
		bool isKeepAlive();
		std::string processGet (bool listDir);
		std::string processHead ();
		std::string processOptions ();
		std::string processTrace ();
		std::string processPost ();
		std::string processPut ();
		std::string processDelete ();
		std::string processVersion ();
		
		std::string mountResponseHeader (int rcode, std::string type, long int lastmod, int size);
};

