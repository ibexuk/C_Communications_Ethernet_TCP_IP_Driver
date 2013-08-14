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
//HTTP HOST C CODE HEADER FILE




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//#######################################
//##### NUMBER OF SOCKETS AVAILABLE #####
//#######################################
//Set using the HTTP_NO_OF_AVAILABLE_SOCKETS define below






//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef HTTP_C_INIT		//(Do only once)
#define	HTTP_C_INIT


//----- HTTP FILE INCLUDE OPTIONS -----
//(Only 1 of these should be left un-comented)
#define	HTTP_USING_C_FILES					//Comment out if not using web files included as part of the source code
//#define	HTTP_USING_BINARY_FILES				//Comment out if not using web files stored in external memory
//#define	HTTP_USING_FILING_SYSTEM			//Comment out if not using web files stored in external memory controlled by a filing system (such as FAT)

//----- CONFIGURATION OPTIONS -----
#define	HTTP_ACCEPT_POST_REQUESTS			//Comment out if you don't want to accept form POST requests (this will reduce memory size).  GET form inputs are still accepted

#define	HTTP_NO_OF_AVAILABLE_SOCKETS		2		//The number of TCP sockets that HTTP should open to listen for new connections (i.e. the maximum
													//number of simultanious http connections.  Value must be <= TCP_NO_OF_AVAILABLE_SOCKETS.  At least 2 is recomended.

#define HTTP_MAX_FILENAME_LENGTH			21		//The maximum length of a http request filename including any sub directory, excluding file extension (including a terminating null - smaller value reduces ram requirement)
#define HTTP_MAX_INPUT_NAME_LENGTH			16		//The maximum number of characters permitted for a single GET or POST input name field (including a terminating null - smaller value reduces ram requirement)
#define HTTP_MAX_INPUT_VALUE_LENGTH			24		//The maximum number of characters permitted for a single GET or POST input value field (including a terminating null - smaller value reduces ram requirement)
#define HTTP_MAX_OUTPUT_VARIABLE_NAME_LENGTH 21		//The maximum number of characters that will be stored for a html dynamic output vairable name (including a terminating null - smaller value reduces ram requirement)



//---------------------------------------------------------------
//----- FUNCTION TO OPTIONALLY REJECT REQUESTS FROM CLIENTS -----
//---------------------------------------------------------------
//The function name in your applicaiton that will be called every time a HEAD, GET or POST request is received from an HTTP client to allow your your
//application to decide if the request should be serviced or denied.  This is useful for applications where you want to only provide HTTP to certain
//clients based on their MAC or IP address, or where you want the to option to effectively disconnect clients after some form of initial sign in page.
//If you don't want this funnctionality simply comment this define out.
#define HTTP_AUTHORISE_REQUEST_FUNCTION	process_http_authorise_request

//Your function definition needs to be:
//		BYTE my_function_name (BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number)
//requested_filename
//	Pointer to a null terminated string containing the filename that will be returned to the client after the driver has finished reading
//	all of the request. Your application may alter this if desired (max length = HTTP_MAX_FILENAME_LENGTH).  Can be ignored if you wish.
//requested_filename_extension
//	Pointer to 3 byte string containing the filename extension.  Your application may alter this if desired or it can be ignored.
//tcp_socket_number
//	Allows your application to identify a user by their mac or IP address (e.g. tcp_socket[tcp_socket_number].remote_device_info.ip_address).
//	Can be ignored if you wish.
//Return
//	1 to authorise the request (http server will process and then respond)
//	0 to reject the request (http server will send a 400 Bad Request response)
//You need to #include the file that contains the function definition at the top of eth-http.c for the compiler

//--------------------------------------------
//----- FUNCTION TO PROVIDE DYNAMIC DATA -----
//--------------------------------------------
//The function name in your application that will provide the dynamic html for .htm files being transmitted which contain the special tilde ~my_varaible_name-
//dynamic data markers.  Comment out if not required.
#define	HTTP_DYNAMIC_DATA_FUNCTION		process_http_dynamic_data

//Your function definition needs to be:
//		BYTE *my_function_name (BYTE *variable_name, BYTE tcp_socket_number)
//variable_name
//	Pointer to a null terminated string containing the varaible name (in the source HTML) between the tilde '~' and '-' characters.
//tcp_socket_number
//	Included in case it is helpful for your application to identify a user (e.g. by their mac or IP address,  e.g. tcp_socket[tcp_socket_number].remote_device_info.ip_address).
//	Can be ignored if you wish.
//Return
//	A pointer to the start of a null termianted string which contains the string to be transmitted (max length 100 characters).
//You need to #include the file that contains this function definition at the top of eth-http.c for the compiler

//------------------------------------------
//----- FUNCTION TO PROCESS HTTP INPUT -----
//------------------------------------------
//The function name in your application that will receive and process http inputs (typically but not necessarily from forms).  Comment out if not required.
#define	HTTP_PROCESS_INPUT_FUNCTION			process_http_inputs

//Your function definition needs to be:
//		void my_function_name (BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number)
//input_name
//	Pointer to a null terminated string containing the input name sent by the client (i.e. the name of the form item in your HTML page)
//input_value
//	Pointer to a null terminated string containing the value returned for this item
//requested_filename
//	Pointer to a null terminated string containing the filename that will be returned to the client after the driver has finished reading all of the input
//	data.  Your application may alter this if desired (max length = HTTP_MAX_FILENAME_LENGTH).  Can be ignored if you wish.
//requested_filename_extension
//	Pointer to 3 byte string containing the filename extension.  Your application may alter this if desired or it can be ignored.
//tcp_socket_number
//	Included in case it is helpful for your application to identify a user (e.g. by their mac or IP address, tcp_socket[tcp_socket_number].remote_device_info.ip_address)
//	Can be ignored if you wish.
//You need to #include the file that contains the function definition at the top of eth-http.c for the compiler


//---------------------------------------------------
//----- FUNCTIONS TO HANDLE MULTIPART FORM DATA -----
//---------------------------------------------------
//If you enable HTTP_ACCEPT_POST_REQUESTS to allow POST requests to be received you need to provide these functions in your application to handle POST
//transfers from clients
#ifdef HTTP_ACCEPT_POST_REQUESTS		//<< This section only relevant if this define is enabled

//----- PROCESS MULTIPART HEADER FOR NEXT SECTION FUNCTION -----
#define	HTTP_POST_MULTIPART_HEADER_FUNCTION 	process_http_multipart_form_header

//Your function definition needs to be:
//	void my_function_name (CONSTANT BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number)
//input_name
//	Pointer to the null termianted header name string
//input_value
//	Pointer to the null termianted value string (converted to lowercase)
//requested_filename
//	Pointer to the null termianted string containing the filename, in case its useful
//requested_filename_extension
//	Pointer to the 3 byte string containing the filename extension, in case its useful
//tcp_socket_number
//	Included in case it is helpful for your application to identify a user (e.g. by their mac or IP address, tcp_socket[tcp_socket_number].remote_device_info.ip_address)
//
//This function is called for each header found for a new multipart section, of a multipart form post
//It will be called 1 or more times, and this signifies that when HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION is next called it will
//be with the data for this new section of the multipart message (i.e. any call to this function means your about to receive data
//for a new multipart section, so reset whatever your application needs to reset to start dealing with the new data).
//The following are the possible values that this function can be called with (always lower case):-
//	content-disposition		Value will be 'form-data' or 'file'
//	name					Name of the corresponding form control
//	filename				Name of the file when content-disposition = file (note that the client is not requried to
//							provide this, but usually will).  May or may not include full path depending on browser.
//							If its important to you to read filename with potentially long paths ensure you set
//							HTTP_MAX_POST_LINE_LENGTH to a value that not cause the end to be cropped off long paths.
//	content-type			Value dependant on the content.  e.g. text/plain, image/gif, etc.
//							If not present then you must assume content-type = text/plain; charset=us-ascii

//----- RECEIVE EACH BYTE OF THIS MULTIPART SECTION -----
#define	HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION		process_http_multipart_form_data

//Your function definition needs to be:
//	void my_function_name (BYTE data)
//This function is called with each decoded byte in turn of a multipart section.  The data you get
//here is the same as the data submitted by the user (the driver deals with all decoding).

//----- END OF MULTIPART SECTION REACHED FUNCTION -----
#define	HTTP_POST_LAST_MULTIPART_DONE_FUNCTION		process_http_multipart_form_last_section_done

//Your function definition needs to be:
//	void my_function_name (void)
//This function is called after the last byte has been received for a multipart section to allow you to carry out any
//operations with the data just received before the next multipart section is started or the end of the form post.
#endif


//---------------------------------------------------------
//----- FUNCTION IF USING BINRY STORAGE OF HTTP FILES -----
//---------------------------------------------------------
#ifdef HTTP_USING_BINARY_FILES		//<< This section only relevant if this define is enabled

//----- RETURN NEXT BYTE OF FILE -----
#define	HTTP_BINARY_FILE_NEXT_BYTE		process_http_binary_file_next_byte

//Your function definition needs to be:
//	BYTE my_function_name (DWORD address)
//address
//	The address of the byte to return, with address 0 being the first byte of the binary file the Web Pages Converter application generated.
//	This value is usually incremented by 1 after each call of this funtion, but will be changed back to a previous value if a TCP packet is
//	lost and is being resent.
//Returns
//	The next byte of the file
#endif


//-------------------------------------------------------------------------------
//----- FUNCTIONS IF USING EXTERNAL FILING SYSTEM FOR STORAGE OF HTTP FILES -----
//-------------------------------------------------------------------------------
#ifdef HTTP_USING_FILING_SYSTEM		//<< This section only relevant if this define is enabled

//----- FIND REQUESTED FILE -----
#define HTTP_EXTERNAL_FILE_FIND_FILE			process_http_find_file

//Your function definition needs to be:
//	BYTE my_function_name (BYTE *request_filename, BYTE *request_file_extension, DWORD *file_size, DWORD *our_pointer_variable)
//request_filename
//	Null terminated string containg the filename.  If file is in a subdirectory string will contain the directory name and the '/' character will have been converted to '_'
//request_file_extension
//	3 byte array containing the filename extension
//file_size
//	This variable must be loaded with the file size in bytes if the file is found
//our_pointer_variable
//	This DWORD variable needs to be a pointer to the first byte of the file.  It may be used as desired by the external file system but
//	may only be written now and will be incremented each time a new byte is requested using HTTP_EXTERNAL_FILE_NEXT_BYTE.  In the event
//	of a TCP packet being lost it will be adjusted back to a previous value so that the contents of a previous packet may be sent.
//	Therefore the external file system may store any value it like in this DWORD variable, but after this function it can only read the
//	value and must return the correct byte being requested.  (Bear in mind that several files may need to be returned to a single or
//	multiple http clients at the same time so this pointer must incorporate the means for the external driver to return the right byte
//	from the right file.
//Returns
//	0 = file not found
//	1 = file found

//----- RETURN NEXT BYTE OF FILE -----
#define	HTTP_EXTERNAL_FILE_NEXT_BYTE		process_http_file_next_byte

//Your function definition needs to be:
//	BYTE my_function_name (DWORD our_pointer_variable)
//our_pointer_variable
//	The pointer to the byte to return.  This value is usually incremented by 1 after each call of this funtion, but will be changed back
//	to a previous value if a TCP packet is lost and is being resent.  Read only
//Returns
//	The next byte of the file
#endif



//GENERAL DEFINES
#define	HTTP_NO_ACTIVITY_TIMEOUT_TIME		6000	//If socket that is connected to a client is inactive for # x 10mS then disconnect the client (error
													//has occured or client is not behaving correctly)
#define	HTTP_MAX_POST_LINE_LENGTH			40		//Maximum characters of a header or body line we buffer when looking for pre data block headers (if
													//too short we will miss header lines we need)
#define	HTTP_PORT							80		//TCP port used for http


typedef struct _HTTP_SOCKET_INFO
{
	BYTE sm_http_state;									//The current state of this socket
	BYTE tcp_socket_id;									//The ID of the TCP socket this HTTP socket is using
	DWORD last_activity_time;							//Timer since last client activity on this socket (used to close the connection after no activity if necessary)
	BYTE response_content_type;
	DWORD response_bytes_remaining;						//The number of bytes of file data still to be sent in response to a client file request
	#ifdef HTTP_USING_C_FILES
		CONSTANT BYTE *response_next_byte_address;		//The address of the next file data byte to be sent in response to a client file request (when storing files in the C source code)
	#endif
	#ifdef HTTP_USING_BINARY_FILES
		DWORD response_next_byte_address;				//The address of the next file data byte to be sent in response to a client file request (when storing files in external bianry memory)
	#endif
	#ifdef HTTP_USING_FILING_SYSTEM
		DWORD response_next_byte_address;				//The address of the next file data byte to be sent in response to a client file request (when storing files in external bianry memory)
	#endif
	WORD file_bytes_sent_last_time;						//The number of file data bytes we sent in the last response which we will use if we need to re-send the last packet. Special values:
														//0xffff = not started response yet, 0xfffe = error response was sent last time
} HTTP_SOCKET_INFO;


//HTTP SOCKET STATE MACHINE STATES
typedef enum _HTTP_SOCKET_STATE
{
	HTTP_WAITING_FOR_CONNECTION,		//Socket is available for a new client
	HTTP_CONNECTED_TO_CLIENT,			//Socket has a TCP connection to a client
	HTTP_PROCESSING_POST,				//Processing a multi packet post request from a client
	HTTP_START_HEAD_RESPONSE,			//Start sending response to a client request that will only include headers (in response to a HEAD request)
	HTTP_START_RESPONSE,				//Start sending response to a client request
	HTTP_CONTINUE_RESPONSE,				//Send next packet of file data to a client request
	HTTP_RETURN_BAD_REQUEST,			//We need to return message: 400 - BAD REQUEST
	HTTP_RETURN_NOT_FOUND,				//We need to return message: 404- NOT FOUND
	HTTP_RETURN_NOT_IMPLEMENTED,		//We need to return message: 501 - REQUEST IS NOT IMPLEMENTED
	HTTP_RETURN_SERVICE_UNAVAILABLE,	//We need to return message: 503 - SERVICE UNAVAILABLE
	HTTP_CLOSE_CONNECTION				//We have finished sending a response that needs the TCP connection to be closed on completion
} HTTP_SOCKET_STATE;


//HTTP REQUEST TYPES
typedef enum _HTTP_REQUEST_TYPE
{
	HTTP_REQUEST_TYPE_NULL,
	HTTP_REQUEST_TYPE_GET,
	HTTP_REQUEST_TYPE_HEAD,
	HTTP_REQUEST_TYPE_POST
} HTTP_REQUEST_TYPE;


#define	HTTP_ENCODING_NONE					0
#define	HTTP_ENCODING_QUOTED_PRINTABLE		1
#define	HTTP_ENCODING_BASE64				2

#endif



#ifdef HTTP_C
//----- HTTP RESPONSE CONTENT TYPES -----
typedef enum _HTTP_CONTENT_TYPE
{
	HTTP_CONTENT_TEXT_PLAIN,
	HTTP_CONTENT_TEXT_HTML,
	HTTP_CONTENT_IMAGE_GIF,
	HTTP_CONTENT_IMAGE_JPEG,
	HTTP_CONTENT_IMAGE_PNG,
	HTTP_CONTENT_TEXT_XML,
	HTTP_CONTENT_TEXT_CSS,
	HTTP_CONTENT_AUDIO_XWAVE,
	HTTP_CONTENT_UNKNOWN
} HTTP_CONTENT_TYPE;

CONSTANT BYTE http_content_type_file_extensions[] = {
	't', 'x', 't', HTTP_CONTENT_TEXT_PLAIN,
	'h', 't', 'm', HTTP_CONTENT_TEXT_HTML,
	'c', 'g', 'i', HTTP_CONTENT_TEXT_HTML,
	'g', 'i', 'f', HTTP_CONTENT_IMAGE_GIF,
	'j', 'p', 'g', HTTP_CONTENT_IMAGE_JPEG,
	'p', 'n', 'g', HTTP_CONTENT_IMAGE_PNG,
	'x', 'm', 'l', HTTP_CONTENT_TEXT_XML,
	'c', 's', 's', HTTP_CONTENT_TEXT_CSS,
	'w', 'a', 'v', HTTP_CONTENT_AUDIO_XWAVE
};
#endif



//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef HTTP_C				//(Defined only by associated C file)
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
void http_process_rx (BYTE socket_number);
void http_setup_response (BYTE socket_number, BYTE request_type, BYTE *request_filename, BYTE *request_file_extension);
void http_transmit_next_response_packet (BYTE socket_number, BYTE resend_last_packet);
void http_transmit_error_response (BYTE socket_number, BYTE resend_last_packet);
BYTE http_process_inputs (BYTE *requested_filename, BYTE *requested_file_extension, BYTE socket_number, BYTE request_is_post_method);
BYTE http_read_next_byte_to_buffer (BYTE *data_buffer);
void http_send_const_string (CONSTANT BYTE *string_to_send);

#ifdef HTTP_ACCEPT_POST_REQUESTS
void http_process_post_message_body (BYTE socket_number);
BYTE http_process_post_check_for_multipart_header (CONSTANT BYTE *header_to_find, BYTE string_length, BYTE socket_number);
void http_process_post_decode_next_data_byte (BYTE data);
BYTE http_convert_base64_to_bits (BYTE character);
#endif


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
void http_initialise (void);
void process_http (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern void http_initialise (void);
extern void process_http (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef HTTP_C				//(Defined only by associated C file)
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
HTTP_SOCKET_INFO http_socket[HTTP_NO_OF_AVAILABLE_SOCKETS];
BYTE sm_http_socket;

#ifdef HTTP_ACCEPT_POST_REQUESTS
BYTE http_post_filename[HTTP_MAX_FILENAME_LENGTH];
BYTE http_post_file_extension[3];
BYTE http_post_is_multipart_type;
BYTE http_post_line_buffer[HTTP_MAX_POST_LINE_LENGTH];
BYTE http_post_line_length;
BYTE http_post_leading_spaces_passed;
DWORD http_post_content_bytes_remaining;
BYTE http_post_boundary_length;
BYTE http_post_boundary_string[70];
BYTE http_post_boundary_compare_length;
BYTE http_post_reading_data;
BYTE http_post_encoding;
BYTE http_post_decode_count;
BYTE http_post_decoder_buffer[3];
#endif


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
#ifdef HTTP_C				//(Defined only by associated C file)

CONSTANT BYTE http_content_length[] = {"content-length:"};							//Must be lower case for receive compare
CONSTANT BYTE http_content_type[] = {"content-type:"};								//Must be lower case for receive compare
CONSTANT BYTE http_multipart_form_data[] = {"multipart/form-data"};				//Must be lower case for receive compare
CONSTANT BYTE http_content_disposition[] = {"content-disposition:"};				//Must be lower case for receive compare
CONSTANT BYTE http_name_equals[] = {"name="};										//Must be lower case for receive compare
CONSTANT BYTE http_filename_equals[] = {"filename="};								//Must be lower case for receive compare
CONSTANT BYTE http_content_transfer_encoding[] = {"content-transfer-encoding:"};	//Must be lower case for receive compare
CONSTANT BYTE http_quoted_printable[] = {"quoted-printable"};						//Must be lower case for receive compare
CONSTANT BYTE http_base64[] = {"base64"};											//Must be lower case for receive compare

CONSTANT BYTE http_response_text_plain[] = {"text/plain\r\n"};
CONSTANT BYTE http_response_text_html[] = {"text/html\r\n"};
CONSTANT BYTE http_response_image_gif[] = {"image/gif\r\n"};
CONSTANT BYTE http_response_image_jpeg[] = {"image/jpeg\r\n"};
CONSTANT BYTE http_response_image_png[] = {"image/png\r\n"};
CONSTANT BYTE http_response_text_xml[] = {"text/xml\r\n"};
CONSTANT BYTE http_response_text_css[] = {"text/css\r\n"};
CONSTANT BYTE http_response_audio_xwave[] = {"audio/x-wave\r\n"};
CONSTANT BYTE http_response_application_octetstream[] = {"application/octet-stream\r\n"};

CONSTANT BYTE http_response_200[] = {"HTTP/1.1 200 OK\r\nConnection: Close\r\n"};
CONSTANT BYTE http_response_400_bad_request[] = {"HTTP/1.1 400 Bad Request\r\nConnection: Close\r\n\r\n400 Bad Request: Could not process your request\r\n"};
CONSTANT BYTE http_response_404_not_found[] = {"HTTP/1.1 404 Not Found\r\nConnection: Close\r\n\r\n404 Not Found: Could not find requested file\r\n"};
CONSTANT BYTE http_response_501_not_implemented[] = {"HTTP/1.1 501 Not Implemented\r\nConnection: Close\r\n\r\n501 Not Implemented: The requested method is not supported\r\n"};
CONSTANT BYTE http_response_503_unavailable[] = {"HTTP/1.1 503 Service Unavailable\r\nConnection: Close\r\n\r\n503 Service Unavailable: Server currently too busy - please try again\r\n"};

#endif




