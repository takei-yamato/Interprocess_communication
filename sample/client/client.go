package main

import (
	"fmt"
	"net"
)

// -----------------------------------------------------------
// 入力受付
// -----------------------------------------------------------
func inputWorker(con net.Conn, ch chan<- string) {
	for {
		fmt.Printf("文字を入力してください\n")

		// 入力文字列の送信
		var str string
		fmt.Scanf("%s\n", &str)

		// サーバーに文字を送信
		con.Write([]byte(str))

		// end と入力すると終了
		if str == "end" {
			// 終了したことをチャネルで通知する
			ch <- str
			break
		}
	}
}

// -----------------------------------------------------------
// メイン関数
// -----------------------------------------------------------
func main() {
	// サーバーとの接続を試みる
	// ※ここでは 8080 ポートを利用していますが、任意のポート番号に変更してください
	con, err := net.Dial("tcp", "localhost:8080")
	if err != nil {
		return
	}

	// 終了処理の遅延呼び出し
	defer con.Close()

	// 入力処理はゴルーチンで分離
	ch := make(chan string)
	go inputWorker(con, ch)

	// チャネルの情報を元に終了するまでブロック
	if "end" == <-ch {
		fmt.Printf("クライアントの処理を終了します\n")
	}
}
