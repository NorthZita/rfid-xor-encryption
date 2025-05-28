<?php
function encrypt(string $plainText, string $key) : string {
    $output = "";//string awal
    $keyPos = 0;
    //Looping sebanyak jumlah karakter yang akan di encrypt
    for ($p = 0; $p < strlen($plainText); $p++) { 
      if ($keyPos > strlen($key) - 1) {
        $keyPos = 0;
      }
      $char = $plainText[$p] ^ $key[$keyPos];//XOR Key
      $bin = str_pad(decbin(ord($char)), 8, "0", STR_PAD_LEFT); //konversi Hasil XOR (decimal) --> binary
  
      $hex = dechex(bindec($bin));// konversi binary --> decimal --> hexa (chiper text)
      $hex = str_pad($hex, 2, "0", STR_PAD_LEFT);//untuk menambah karakter di sebelah kiri data jika kurang 
      $output .= strtoupper($hex);//karakter dibuat UPPER CASE
      $keyPos++;
    }
    return $output;
  }
  
?>