jam -j24 && start "server" cmd /k dist\server.exe 1337 && start "client1" cmd /k dist\client.exe localhost 1337 && start "client2" cmd /k dist\client.exe localhost 1337
