<?php
$access_token = 'PJAmOc1EG7sy/3rVTvDBDH62ClsUwFaMQh/c69Xs5UTNGoCEuMOKeyDuRaQGEtDeKe5st1YBFm5g01xA2EzM8Qo0ZS2SVdgZCjY+n4M5svW2GffKSqUEyDojpNObkt7maHff5gxnNbBPUuK79zR2AgdB04t89/1O/w1cDnyilFU=';    //PUT LINE token ID at "Channel access token (long-lived)"
$url = 'https://api.line.me/v1/oauth/verify';
$headers = array('Authorization: Bearer ' . $access_token);

$ch = curl_init($url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);

$result = curl_exec($ch);
curl_close($ch);

echo $result;
?>
