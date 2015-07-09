//Include Files.
//Custom headers
#include "httpHeaders.h" 

//C++ headers 
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <list>

//C headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace std;

//Funcao que busca e compara a extensao de um arquivo com uma string
bool testType(string recurso, string type);

string getContentType(string recurso);


/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
requestHeader::requestHeader  () {

}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
void requestHeader::refresh () {
	int i;
	op = M_GET;
	version = "";
	resource = "";
	for (i = 0; i < parameters.size(); i++) {
		parameters[i].values.clear();
	}
	parameters.clear();
	//parameters.erase(parameters.begin(), parameters.end());
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
bool requestHeader::isKeepAlive () {
	int size, i;
	list<string>::iterator it;
	bool found = false;
	size = parameters.size();
	
	//Look for Connection header
	for(i = 0; i < parameters.size() ; i++) {
		cout << parameters[i].name << endl;
		if(!parameters[i].name.compare("Connection") ) break;
	}
	
	//In case it exists, look for the keep-alive parameter on the value list
	if(i < parameters.size()) {
		for (it = parameters[i].values.begin(); it != parameters[i].values.end(); it++)
			if(!(*it).compare("keep-alive") || !(*it).compare("Keep-Alive")) found = true;
	}
	return found;
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
std::string requestHeader::processGet (bool listDir) {
	string stdInFile = "index.html";	//Open this file in directory passed
	string resPath;							//The full resource path
	struct stat rinfo;						//Holds info on the resource
	int fd;										//File descriptor to open and read the resource
	int i, j;									//Counters
	char auxBuf[1024];						//Auxiliary buffer to read a file
	ostringstream response;					//Used to build the responseHeader
	struct dirent **namelist;				//Used to list directory
	ostringstream auxResBldr;				//Used to build directory list string

	//Create full resource path
	resPath = "./Web" + resource;
	
	//Check if resource exists
	if(stat(resPath.data(), &rinfo) == -1) {
    	response << mountResponseHeader(404, "", 0, 0);
    	return response.str();
	}
	//Check if resource is file
	if ((rinfo.st_mode & S_IFMT) == S_IFREG) {
		//Check if its an executable
		if(!access(resPath.data(), X_OK));
		
		//Check if it ran be read
		else if(!access(resPath.data(), R_OK));
		
		else {
			response << mountResponseHeader(403, "", 0, 0);
			return response.str();
		}
	}
	
	//Check if resource is directory
	else if ((rinfo.st_mode & S_IFMT) == S_IFDIR) {
      //Check for exec permission, return 403 if forbidden
		if(access(resPath.data(), X_OK)) {
			response << mountResponseHeader(403, "", 0, 0);
			return response.str();
		}
			
		//Use file specified by stdInFile 
		resPath += stdInFile;
		
		//Check if file specified by stdInFile exists and is readable. If so, reads it
		if((stat(resPath.data(), &rinfo) != -1) && (access(resPath.data(), R_OK) != -1));
      else {
			//Go back to using the Directory	
			resPath = "./Web" + resource;
			
			//List directory contents if allowed
			if(listDir) {
				j = scandir(resPath.data(), &namelist, NULL, alphasort);
				if(j < 0) exit(1);
				auxResBldr << resource + ":\n"; 
				for(i = 0; i < j; i++){
					auxResBldr << namelist[i]->d_name << "\n";		
					free(namelist[i]);
				}
    			response << mountResponseHeader(200, "dir", rinfo.st_mtime, auxResBldr.str().size());
				response << auxResBldr.str();
				return response.str();
			}
			else {
				response << mountResponseHeader(404, "", 0, 0);
				return response.str();
			}
		}
	}
	//Not a file and not a 
	else {
		response << mountResponseHeader(404, "", 0, 0);
    	return response.str();
	}
   response << mountResponseHeader(200, getContentType(resPath), rinfo.st_mtime, rinfo.st_size);
	fd = open(resPath.data(), O_RDONLY);
	while((i = read(fd,auxBuf, 1024)) > 0) {
		for(j = 0; j < i; j++)
			response << auxBuf[j];
			cout << auxBuf[j];
	}
	close(fd);
	return response.str();
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
std::string requestHeader::processHead () {
	return string("test");
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
std::string requestHeader::processOptions () {
	string resPath = "./Web/options.txt";
	struct stat oinfo;
	int fd;										//File descriptor to open and read the resource
	int i, j;									//Counters
	char auxBuf[1024];						//Auxiliary buffer to read a file
	ostringstream response;					//Used to build the responseHeader
	
	stat(resPath.data(), &oinfo);
	response << mountResponseHeader(200, getContentType(resPath), oinfo.st_mtime, oinfo.st_size);

	fd = open(resPath.data(), O_RDONLY);
	while((i = read(fd,auxBuf, 1024)) > 0) {
		for(j = 0; j < i; j++)
			response << auxBuf[j];
			cout << auxBuf[j];
	}
	close(fd);
	return response.str();
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
std::string requestHeader::processTrace () {
	return string("test");
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
std::string requestHeader::processPost () {
	return string("test");
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
std::string requestHeader::processPut () {
	return string("test");
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
std::string requestHeader::processDelete () {
	return string("test");
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Implements the VERSION method. Not yet implemented
 *
 * RETURNS: 
 *	A string containing the response header followe by the requested content or the associated error
 *HTML page
 * 
 */
std::string requestHeader::processVersion () {
	return string("test");
}

/***************************************************************************************************
 *	MountResponseHeader
 *
 * DESCRIPTION: 
 *	Mounts the apropriate header given a response code, the content type, the date
 *of last modification and the content size. If an error code is passed, auto-attaches
 *the appropriated HTML Error page after the Header.
 *
 * RETURNS: A string containing the header and possible associated error HTML page
 * 
 */
std::string requestHeader::mountResponseHeader (int rcode, string type, long int lastmod, int size) {
	string server = "localhost";
	ostringstream response;
	time_t ttime;
	struct tm *tmtime;
	char date[128];
	bool err;
	struct stat errinfo;
	ostringstream errPath;
	char errBuf[1024];
	int fd;
	int i, j;
	
	//Begin
	
	//Create first line
	response << "HTTP/1.1 " << rcode << " "; //Assume always HTTP1.1
	switch (rcode) {	//Gen code message
	case 200:
		response << "OK";
		err = false;
		break;
	case 401:
		response << "Unauthorized";
		err = true;
		break;
	case 403:
		response << "Forbidden";
		err = true;
		break;
	case 404:
		response << "Not Found";
		err = true;
		break;
	case 405:
		response << "Method Not Allowed";
		err = true;
		break;
	case 406:
		response << "Not Acceptable";
		err = true;
		break;
	case 408:
		response << "Request Timeout";
		err = true;
		break;	
	case 500:
		response << "Internal Server Error";
		err = true;
		break;
	case 503:
		response << "Service Unavailable";
		err = true;
		break;	
	default:
		response << "Internal Server Error";
		rcode = 500;
		err = true;
		break;
	}
	response << cr << lf; //End line
	
	//Get date
	time(&ttime);
	tmtime = gmtime(&ttime);
	strftime(date, 128, "%a, %d %b %Y %T GMT", tmtime);
	
	//Insert Server and Date
	response	<< "Server: " << server << cr << lf
				<< "Date: " << date << cr << lf;
				
	//If not a 200 response load apropriate error file info			
	if(err) {
		errPath << "./Web/" << rcode << ".html";
		stat(errPath.str().data(), &errinfo);
		type = getContentType(errPath.str());
		lastmod = errinfo.st_mtime;
		size = errinfo.st_size;
	}
	
	//Format date of last mod for printing
	tmtime = gmtime(&lastmod);
	strftime(date, 128, "%a, %d %b %Y %T GMT", tmtime);
	
	//Insert other parameters
	response << "Content-Type: " << type << cr << lf
				<< "Last-Modified: " << date << cr << lf
				<< "Content-Length: " << size << cr << lf
				<< cr << lf;
				
	//If not a 200 response attach apropriate error file			
	if(err) {
		fd = open(errPath.str().data(), O_RDONLY);
		while((i = read(fd, errBuf, 1024)) > 0)
	 		for(j = 0; j < i; j++) 
	 			response << errBuf[j];
		close(fd);
	}			
	return response.str();
}


/****************************************************************************************************
 *	getContentType
 *
 * DESCRIPTION: 
 *	Analizes a resource's type
 *
 * RETURNS: A string containing type of the resource
 *
 */
string getContentType(string resource) {
  if(testType(resource, string(".html"))) return (string("text/html"));
  if(testType(resource, string(".htm")))	return (string("text/html"));
  if(testType(resource, string(".xml")))	return (string("text/xml"));
  if(testType(resource, string(".xhtml")))return (string("application/xhtml+xml"));
  if(testType(resource, string(".css")))	return (string("text/css"));
  if(testType(resource, string(".gif")))	return (string("image/gif"));
  if(testType(resource, string(".jpeg"))) return (string("image/jpeg"));
  if(testType(resource, string(".jpg")))	return (string("image/jpeg"));
  if(testType(resource, string(".png")))	return (string("image/png"));
  if(testType(resource, string(".mpeg"))) return (string("video/mpeg"));
  if(testType(resource, string(".mov")))	return (string("video/quicktime"));
  if(testType(resource, string(".wav")))	return (string("audio/x-wav"));
  if(testType(resource, string(".js")))	return (string("application/javascript"));
  if(testType(resource, string(".pdf")))	return (string("application/pdf"));
  if(testType(resource, string(".swf")))	return (string("application/x-shockwave-flash"));
  return (string("text/plain"));
}

bool testType(string recurso, string type) {
  int rlen = recurso.length();
  int tlen = type.length();
  size_t pos = recurso.rfind(type);
  // a ocorrencia deve casar com a ultima extensao do nome
  if(pos == string::npos || pos != (rlen - tlen)) return false;
  return true;
}



