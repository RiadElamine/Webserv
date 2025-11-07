<?php
    // logout.php
    $location = "/login/";
    $statusCode = 302;
    // protected logout page
    session_save_path("/tmp");

    $cookie_env = getenv("HTTP_COOKIE");
    if ($cookie_env) {
        parse_str(str_replace(';', '&', $cookie_env), $cookies);
        if (is_array($cookies)) {
            foreach ($cookies as $name => $value) {
                $_COOKIE[$name] = $value;
                fwrite(STDERR, "Cookie: " . $name . " = " . $value . "\n");
            }
        }
    }

    session_start();

    if (!isset($_SESSION['logged_in']) || !$_SESSION['logged_in']) {
        fwrite(STDERR, "No user is currently logged in.\n");
    }
    else {
        fwrite(STDERR, "User logged out successfully.\n");
    }

    $GLOBALS['location'] = "/login/";
    $GLOBALS['statusCode'] = 302;
    if (isset($_COOKIE[session_name()])) {
        echo "Set-Cookie: " . session_name() . "=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly\r\n";
    }
    session_destroy();
    
    function getHeader($bodyLength) {
        // Manually print CGI-style headers
        if ($GLOBALS['statusCode'] !== 200)
            $bodyLength = 0;
        $headers = array(
            "Status: " . $GLOBALS['statusCode'],
            "Location: " . $GLOBALS['location'],
            "Content-Type: text/html",
            "Content-Length: " . $bodyLength,
            "Connection: close",
            "Server: WebServer/1.1.0"
        );
        // Join headers with CRLF and add an empty line before body
        echo implode("\r\n", $headers) . "\r\n\r\n";
    }
    function main() {
        getHeader(0);
    }
    main();
?>