# C hiddenfile module

### About

Модуль для ядра linux, который регистрирует девайс, доступный только по паролю  
Создан для демонстрации возомжности прятать файл с помощью модификации ядра linux

---

### Quickstart

Загрузка модуля:

```bash
$ make load
```

Выгрузка модуля:

```bash
$ make unload
```

Получение логов ядра:

```bash
$ ./live_logs.sh
```

Создание девайса hiddenfile, где [MAJOR] - полученное в логах зачение major

```bash
$ sudo mknod /dev/hiddenfile c [MAJOR] 0
```

### Usage:

Пример использования девайса на rust

```rust
use std::{
    fs::OpenOptions,
    io::{Read, Write},
};

pub struct HiddenfileAdapter {
    f: std::fs::File,
}

impl HiddenfileAdapter {
    const KEY: &'static str = "4mu7xa3r0wmsl97xrgfpk5ycyprwlezc";

    pub fn new() -> Self {
        let f = OpenOptions::new()
            .read(true)
            .write(true)
            .open("/dev/hiddenfile")
            .unwrap();
        Self { f }
    }
}

impl std::io::Write for HiddenfileAdapter {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        let new_buf: Vec<_> = Self::KEY.bytes().chain(buf.into_iter().copied()).collect();
        self.f.write(new_buf.as_ref())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }
}

impl std::io::Read for HiddenfileAdapter {
    fn read(&mut self, mut buf: &mut [u8]) -> std::io::Result<usize> {
        let mut new_buf = [0u8; 128 + 32];
        new_buf[..32].copy_from_slice(Self::KEY.bytes().collect::<Vec<_>>().as_ref());
        let len = self.f.read(&mut new_buf)?;
        let _ = buf.write(&new_buf[..len]);

        Ok(len)
    }
}

fn main() {
    let mut file = HiddenfileAdapter::new();
    let string = "FooBar";

    file.write(string.as_ref()).unwrap();
    println!("Written to device: {}", string); // Written to device: FooBar

    let mut buf = [0u8; 128];
    let len = file.read(&mut buf).unwrap();
    println!(
        "Read from the device: {:?}", // Read from the device: "FooBar"
        String::from_utf8_lossy(&buf[..len])
    );
}
```
