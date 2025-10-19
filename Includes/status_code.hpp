#ifndef __STATUS_CODE_H_
#define __STATUS_CODE_H_

enum e_StatusCode {
    UNITILAZE = 0,
    Continue = 100,
    OK = 200,
    Created = 201,
    No_Content = 204,
    Moved_Permanently = 301,
    Found = 302,
    Bad_Request = 400,
    Unauthorized = 401,
    Forbidden = 403,
    Not_Found = 404,
    Method_Not_Allowed = 405,
    Request_Timeout = 408,
    Conflict = 409,
    Payload_Too_Large = 413,
    Internal_Server_Error = 500,
    Not_Implemented = 501,
    Bad_Gateway = 502,
    Service_Unavailable = 503,
    Gateway_Timeout = 504
};

enum isSendingStatus {
    Doesnt_fail = -2,
    I_Dont_have_respons = -1,
};
#endif