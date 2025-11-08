<?php
    $location = "/login/";
    $statusCode = 200;

    // loop in $_COOKIE and print all cookies
    session_save_path("/tmp");

    // Set up cookies from environment (for CGI)
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

    // Redirect if already logged in
    if (isset($_SESSION['logged_in']) && $_SESSION['logged_in']) {
        $GLOBALS['location'] = "/dashboard/";
        $GLOBALS['statusCode'] = 302;
    }
    else if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        // Read ALL users from database.csv
        $users_csv = [];
        $csv_file = "../database/database.csv";
        if (file_exists($csv_file) && $file = fopen($csv_file, "r")) {
            while ($line = fgets($file)) {
                $line = trim($line);
                if ($line === '' || strpos($line, '=') === false) {
                    continue;
                }
                list($user, $pass) = explode('=', $line, 2);
                $users_csv[$user] = $pass;
            }
            fclose($file);
        }
        else 
        {
            exit(1);
        }

        // Read raw POST data from stdin
        $post_data = file_get_contents('php://stdin');
        // For application/x-www-form-urlencoded data, parse it
        parse_str($post_data, $parsed_data);
        // Now you can access the data
        $username = $parsed_data['username'] ?? '';
        $password = $parsed_data['password'] ?? '';
        $confirm_password = $parsed_data['confirm_password'] ?? '';
        
        // Validate credentials
        if (!isset($users_csv[$username]) && $password === $confirm_password && $username !== '' && $password !== '') {
            // signup successful
            $_SESSION['username'] = $username;
            $_SESSION['logged_in'] = true;
            $GLOBALS['location'] = "/dashboard/";
            $GLOBALS['statusCode'] = 302;
            echo "Set-Cookie: " . session_name() . "=" . session_id() . "; Path=/; HttpOnly\r\n";
            // Append new user to database.csv
            file_put_contents($csv_file, $username . "=" . $password . "\n", FILE_APPEND | LOCK_EX);
            fwrite(STDERR, "signup successful for user: " . $username . "\n");
        } else {
            // Invalid username or password.
            fwrite(STDERR, "signup failed for user: " . $username . "\n");
            $GLOBALS['statusCode'] = 302;
            $GLOBALS['location'] = "/login/?isSignup=true";
            session_destroy();
        }
    }
    else
    {
        session_destroy();
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


    function main() {

            getHeader(0);
    }

// Entry point
    main();
?>