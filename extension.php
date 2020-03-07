<?php

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
