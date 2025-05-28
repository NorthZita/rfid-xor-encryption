<?php

require_once "encrypt.php"; //Load fungsi enkripsi (XOR) file encrypt.php
require_once "decrypt.php"; //Load fungsi dekripsi (XOR) file decrypt.php

$servername = "localhost"; //IP address server database, bisa diganti dengan IP PC
$dbname = "esp_data"; //nama database yang ada di MYSQL (bisa di cek via phpmyadmin)
$username = "root"; //nama user database untuk akses ke database MYSQL
$password = ""; // password dari user root


$sharedkey = ["ABC123", "BCD234", "FG56", "FG56", "FG56", "FG56", "FG56", "FG56"]; //list key untuk enkripsi
$uid = ""; //inisiasi variable uid

/**********************************************************\
metode komunikasi protokol HTTP yang digunakan oleh ESP HTTP Method
1. POST --> data diembed pada body HTTP request
\**********************************************************/
if ($_SERVER["REQUEST_METHOD"] == "POST") { 
    $uid_enc = test_input($_POST["uid"]); //get POST data yang telah terenkrip dari ESP untuk variable uid
    $user_enc = test_input($_POST["username"]); //get POST data yang telah terenkrip dari ESP untuk variable username
    $pass_enc = test_input($_POST["password"]);
    $nim_enc = test_input($_POST["nim"]);
    $secret_key_enc = test_input($_POST["secret_key"]);
    $key_id = test_input($_POST["sharedkey"]);
    $mykey = $sharedkey[$key_id];
    //Proses dekripsi data dari ESP
    $uid = decrypt($uid_enc,$mykey);
    $user = decrypt($user_enc,$mykey);
    $pass = decrypt($pass_enc,$mykey);
    $nim = decrypt($nim_enc,$mykey);
    $secret_key = decrypt($secret_key_enc,$mykey);
    $conn = new mysqli($servername, $username, $password, $dbname); //Akses ke database MYSQL
    //Query untuk mengecek uid sudah teregistrasi atau belum
    $sql = "SELECT * FROM register WHERE uid = '$uid'"; 
    $result = mysqli_query($conn, $sql);
    $num = mysqli_num_rows($result);
    //Jika uid belum teregistrasi maka uid akan di insert ke dalam database MYSQL
    if ($num==0){
        $sql = "INSERT INTO register (uid, username, nim, password, secret_key, status)
        VALUES ('" . $uid . "', '" . $user . "', '" . $nim . "', '" . $pass . "', '" . $secret_key . "', 'Allowed')";

        $result = mysqli_query($conn, $sql);
        echo "Regisrasi Telah disimpan pada Database";
        $conn->close();
    }
    //Jika uid sudah teregistrasi
    else {
        echo "UID is exist";
    }

}
else {
    echo "No data posted with HTTP POST.";
}

//Template fungsi untuk parsing POST data
function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}