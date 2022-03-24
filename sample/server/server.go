package main

import (
	"fmt"
	"net"
	"os"
)

// 受信ワーカー
func work(con net.Conn) {
	// 終了処理の遅延呼び出し
	defer con.Close()

	for {
		// 送られてきた情報の確認
		buf := make([]byte, 1024)
		n, err := con.Read(buf)

		// 切断検知
		if err != nil {
			fmt.Fprintln(os.Stderr, err.Error())
			break
		}
		// 受信内容出力
		fmt.Printf("%s\n", string(buf[:n]))
	}
}

// メイン関数
func main() {
	// 8080ポートでサーバーを立ち上げる
	listen, err := net.Listen("tcp", "localhost:8080")
	// 終了処理の遅延呼び出し
	defer listen.Close()

	if err != nil {
		fmt.Printf("立ち上げ失敗")
	}

	for {
		// リクエスト開始 and 受け付けるまで待機
		con, _ := listen.Accept()
		go work(con)
	}
}
