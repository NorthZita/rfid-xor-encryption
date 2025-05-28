<?php
require_once "encrypt.php";
require_once "decrypt.php";

$servername = "localhost";
$dbname = "esp_data";
$username = "root";
$sharedkey = ["ABC123", "BCD234", "FG56", "FG56", "FG56", "FG56", "FG56", "FG56"];
$password = "";
$uid = "";


if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $new_secret_key_enc = test_input($_POST["new_secret_key"]);
    $nim_enc = test_input($_POST["nim"]);
    $uid_enc = test_input($_POST["uid"]);
    $key_id = test_input($_POST["sharedkey"]);
    $mykey = $sharedkey[$key_id];
    $uid = decrypt($uid_enc,$mykey);
    $new_secret_key = decrypt($new_secret_key_enc,$mykey);
    $nim = decrypt($nim_enc,$mykey);

    $conn = new mysqli($servername, $username, $password, $dbname);
    
    //Query untuk update synchronized secret baru dari ESP, ketika kartu melakukan tap
    $sql = "UPDATE register SET secret_key = '$new_secret_key' WHERE nim = '$nim'";  

    //Query insert record jika kartu melakukan tap
    $tapping = "INSERT INTO tapping (uid, nim)
    VALUES ('" . $uid . "', '" . $nim . "')";

    // $result = mysqli_query($conn, $sql);
    $result = $conn->query($sql);
    $result2 = $conn->query($tapping);

    $conn->close();

    
}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}