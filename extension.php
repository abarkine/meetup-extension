<?php

use \Tutorial\CurlEasy;

tutorial_curl_version();

var_dump(tutorial_curl_ver());

var_dump(tutorial_curl_escape('Speakap MeetUp'));

var_dump(tutorial_curl_info());

tutorial_hello_world([
    'name' => 'Speakap',
    'greet' => false
]);

tutorial_hello_world([
    'name' => 'Speakap'
]);

tutorial_greet_everyone([
    'Speakap' => 'Hoi',
    'MeetUp' => 'Hi',
    'Asil' => 'Merhaba'
]);

var_dump(CurlEasy::escape('http://developer.speakap.io/#fragment?query1=value1&query2=value1 value2'));

$curlEasy = new CurlEasy('https://www.speakap.com');
$curlEasy = $curlEasy->setOpt(CurlEasy::OPT_URL, 'http://developer.speakap.io');
$curlEasy->perform();

var_dump(tutorial_get_default());

$defaultCurlEasy = new CurlEasy();
$defaultCurlEasy->perform();
