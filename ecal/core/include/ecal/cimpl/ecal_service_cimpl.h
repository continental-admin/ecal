/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

/**
 * @file   ecal_service_cimpl.h
 * @brief  eCAL service c interface
**/

#ifndef ecal_service_cimpl_h_included
#define ecal_service_cimpl_h_included

#include <ecal/cimpl/ecal_service_info_cimpl.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Create a server. 
   *
   * @param service_name_  Service name.
   *
   * @return  Handle to created server or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Server_Create(const char* service_name_);

  /**
   * @brief Destroy a server. 
   *
   * @param handle_  Server handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Server_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Add server method callback.
   *
   * @param handle_     Server handle.
   * @param method_     Service method name.
   * @param req_type_   Method request type (default = "").
   * @param resp_type_  Method response type (default = "").
   * @param callback_   Callback function for server request.
   * @param par_        User defined context that will be forwarded to the request function.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Server_AddMethodCallbackC(ECAL_HANDLE handle_, const char* method_, const char* req_type_, const char* resp_type_, MethodCallbackCT callback_, void * par_);

  /**
   * @brief Remove server method callback.
   *
   * @param handle_  Server handle.
   * @param method_  Service method name.
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Server_RemMethodCallbackC(ECAL_HANDLE handle_, const char* method_);

  /**
   * @brief Retrieve the service name.
   *
   * @param       handle_   Server handle.
   * @param [out] buf_      Pointer to store the server service string.
   * @param       buf_len_  Length of allocated buffer or ECAL_ALLOCATE_4ME if
   *                        eCAL should allocate the buffer for you (see eCAL_FreeMem).
   *
   * @return  Description buffer length or zero if failed.
  **/
  ECALC_API int eCAL_Server_GetServiceName(ECAL_HANDLE handle_, void* buf_, int buf_len_);

  /**
   * @brief Create a client. 
   *
   * @param service_name_  Service name. 
   *
   * @return  Handle to created client or NULL if failed.
  **/
  ECALC_API ECAL_HANDLE eCAL_Client_Create(const char* service_name_);

  /**
   * @brief Destroy a client. 
   *
   * @param handle_  Client handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_Destroy(ECAL_HANDLE handle_);

  /**
   * @brief Change the host name filter for that client instance
   *
   * @param handle_     Client handle. 
   * @param host_name_  Host name filter (empty or "*" == all hosts) 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_SetHostName(ECAL_HANDLE handle_, const char* host_name_);

  /**
   * @brief Call method of this service (none blocking variant with callback). 
   *
   * @param handle_       Client handle. 
   * @param method_name_  Method name. 
   * @param request_      Request message buffer. 
   * @param request_len_  Request message length. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_Call(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_);

  /**
   * @brief Call method of this service (blocking variant with timeout). 
   *
   * @param       handle_        Client handle. 
   * @param       host_name_     Host name.
   * @param       method_name_   Method name.
   * @param       request_       Request message buffer. 
   * @param       request_len_   Request message length. 
   * @param [out] service_info_  Service info struct with additional infos like call state and
   *                             error message.
   * @param [out] response_      Pointer to the allocated buffer for the response message.
   * @param       response_len_  Response message buffer length or ECAL_ALLOCATE_4ME if
   *                             eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Size of response buffer if succeeded, otherwise zero.
  **/
  ECALC_API int eCAL_Client_Call_Wait(ECAL_HANDLE handle_, const char* host_name_, const char* method_name_, const char* request_, int request_len_, struct SServiceInfoC* service_info_, void* response_, int response_len_);

  /**
   * @brief Add server response callback. 
   *
   * @param handle_    Client handle. 
   * @param callback_  Callback function for server response.  
   * @param par_       User defined context that will be forwarded to the callback function.  
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_AddResponseCallbackC(ECAL_HANDLE handle_, ResponseCallbackCT callback_, void* par_);

  /**
   * @brief Remove server response callback. 
   *
   * @param handle_  Client handle. 
   *
   * @return  None zero if succeeded.
  **/
  ECALC_API int eCAL_Client_RemResponseCallback(ECAL_HANDLE handle_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_service_cimpl_h_included*/
