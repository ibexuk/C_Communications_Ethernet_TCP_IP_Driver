/*
IBEX UK LTD http://www.ibexuk.com
Electronic Product Design Specialists
RELEASED SOFTWARE

The MIT License (MIT)

Copyright (c) 2013, IBEX UK Ltd, http://ibexuk.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//Project Name:		TCP/IP DRIVER
//HTTP CLIENT C CODE HEADER FILE




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//Set other defines as required below.






//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef HTTP_CLIENT_C_INIT		//(Do only once)
#define	HTTP_CLIENT_C_INIT


#define	HTTP_CLIENT_MAX_URL_LENGTH			26
#define	HTTP_CLIENT_MAX_FILENAME_LENGTH		26



//-------------------------------------------------
//----- FUNCTION TO PROCESS HTTP CLIENT FILES -----
//-------------------------------------------------
//The function name in your application that will be called with each sequential packet of requested HTTP files:
#define HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION	process_http_client_request

//Your function definition needs to be:
//		void my_function_name (BYTE op_code, DWORD content_length, BYTE *requested_host_url, BYTE *requested_filename)
//op_code
//	0 = error - http client failed.
//	1 = OK.  First section of TCP file data ready to be read.  Function should use the tcp_read_next_rx_byte or tcp_read_rx_array functions to read all of the data from this TCP packet before returning.
//	2 = Next section of TCP file data ready to be read.  Function should use the tcp_read_next_rx_byte or tcp_read_rx_array functions to read all of the data from this TCP packet before returning.
//	0xff = The remote server has closed the connection (this will mark the end of the file if content-length was not provided by the server)
//content_length
//	The file length specified by the server at the start of the response.  Note that the server is not requried to specify this (and many don't) and in this instance the value will be 0xffffffff;
//requested_host_url
//	Pointer to a null terminated string containing the host url that was originally requested
//requested_filename
//	Pointer to a null terminated string containing the filename that was originally requested
//
//Read each data byte in the packet using:
//	while (tcp_read_next_rx_byte(&my_byte_variable))
//	{
//
//This function is always called after a request, either to either indicate the request failed, or with 1 or more packets of file data.  If the request was sucessful the file data packets will be received
//in the correct order and the tcp data may simply be stored as it is read.  The HTTP Client get request headers specify that no encoding may be used by the remote server so the file will be received exactly
//as it is stored on the server.


typedef enum _SM_HTTP_CLIENT
{
	SM_HTTP_CLIENT_IDLE,
    SM_HTTP_CLIENT_RESOLVE_URL_START,
    SM_HTTP_CLIENT_RESOLVE_URL_WAIT,
    SM_HTTP_CLIENT_OPEN_SOCKET,
    SM_HTTP_CLIENT_WAIT_FOR_CONNECTION,
    SM_HTTP_CLIENT_WAIT_FOR_RESPONSE,
    SM_HTTP_CLIENT_WAIT_FOR_RESPONSE_1,
    SM_HTTP_CLIENT_TX_REQUEST,
    SM_HTTP_CLIENT_REQUEST_DISCONNECT,
    SM_HTTP_CLIENT_WAIT_FOR_DISCONNECT,
    SM_HTTP_CLIENT_FAIL
} SM_HTTP_CLIENT;


#endif



//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef HTTP_CLIENT_C			//(Defined only by associated C file)
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
void http_client_send_const_string (CONSTANT BYTE *string_to_send);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE start_http_client_request (CONSTANT BYTE *host_url, CONSTANT BYTE *filename);
void process_http_client (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE start_http_client_request (CONSTANT BYTE *host_url, CONSTANT BYTE *filename);
extern void process_http_client (void);



#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef HTTP_CLIENT_C			//(Defined only by associated C file)
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE http_client_state = SM_HTTP_CLIENT_IDLE;
BYTE http_client_host_url[HTTP_CLIENT_MAX_URL_LENGTH];
BYTE http_client_filename[HTTP_CLIENT_MAX_FILENAME_LENGTH];
BYTE http_client_100ms_timeout_timer;
BYTE http_client_request_ok;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------


#endif




//*****************************
//*****************************
//********** STRINGS **********
//*****************************
//*****************************
#ifdef HTTP_CLIENT_C			//(Defined only by associated C file)

CONSTANT BYTE http_client_request_text_get_start[] = {"GET /"};
CONSTANT BYTE http_client_request_text_get_end[] = {" HTTP/1.0\r\n"};
CONSTANT BYTE http_client_request_text_host_start[] = {"Host: "};
CONSTANT BYTE http_client_request_text_remainder[] = {"\r\nAccept-Encoding: \r\n\r\n"};			//Accept-Encoding blank = server must send using identity encoding (no encoding
																								//is permissible).
																								//End with blank linee

CONSTANT BYTE http_client_content_length_text[] = {"content-length:"};


#endif




