<?php
    $location = "dashboard.html";
    $statusCode = 200;
    // // protected dashboard page
    session_save_path("/tmp");

    $cookie_env = getenv("HTTP_COOKIE"); 
    if ($cookie_env) {
        parse_str(str_replace(';', '&', $cookie_env), $cookies);
        if (is_array($cookies)) {
            foreach ($cookies as $name => $value) {
                $_COOKIE[$name] = $value;
            }
        }
    }

    session_start();

    if (!isset($_SESSION['logged_in']) || !$_SESSION['logged_in']) {
        $GLOBALS['location'] = "/login/";
        $GLOBALS['statusCode'] = 302;
        session_destroy();
    }
    else {
        // User is logged in, show dashboard
        fwrite(STDERR, "User " . $_SESSION['username'] . " accessed the dashboard.\n");
    }

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

    // --- Read the HTML file dynamically ---
    function getPageBody() {
        $path = "../../Design/dashboard/dashboard.html";
        if (!file_exists($path)) {
            exit(1);
        }
        // Load content
        $body = file_get_contents($path);
        if (isset($_SESSION['username'])) {
            $body = str_replace('Guest', htmlspecialchars($_SESSION['username']), $body);
        }
        return $body;
    }

    function main() {
        if ($GLOBALS['statusCode'] === 200)
        {
            $body = getPageBody();
            $len = strlen($body);
            getHeader($len);
            echo $body;
        }
        else
        {
            getHeader(0);
        }
    }

    // Entry point
    main();

?>
