<?php
$servername = "localhost";
$dbname = "esp_data";
$username = "root";
$password = "";
$uid = "";
$conn = new mysqli($servername, $username, $password, $dbname);
$uid = test_input($_GET["uid"]);
$status = test_input($_GET["status"]);

$sql = "UPDATE register SET status = '$status' WHERE uid = '$uid'";
$result = $conn->query($sql);
$conn->close();

echo "Kartu telah di block";
echo "</br>";
echo "<a href=/esp/esp-data.php>Back to Home</a>";

// if ($_SERVER["REQUEST_METHOD"] == "POST") {
//     $new_secret_key = test_input($_POST["new_secret_key"]);
//     $nim = test_input($_POST["nim"]);
//     $uid = test_input($_POST["uid"]);
//     $conn = new mysqli($servername, $username, $password, $dbname);
//     // $sql = "SELECT secret_key FROM register WHERE nim = '$nim'";

//     $sql = "UPDATE register SET secret_key = '$new_secret_key' WHERE nim = '$nim'";  

//     $tapping = "INSERT INTO tapping (uid, nim)
//     VALUES ('" . $uid . "', '" . $nim . "')";

//     // $result = mysqli_query($conn, $sql);
//     $result = $conn->query($sql);
//     $result2 = $conn->query($tapping);

//     $conn->close();

    
// }
// else {
//     echo "No data posted with HTTP POST.";
// }

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}