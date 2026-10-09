# myls - NetBSD ls Command Implementation

- **Đề tài:** Xây dựng chương trình mô phỏng lệnh `ls` trên NetBSD
- **Hệ điều hành:** NetBSD 10.1
- **Ngôn ngữ:** C
- **Môn học:** Lập trình hệ thống
- **Mã nguồn:** `main.c`, `ls.c`, `fileinfo.c`

## 1. Giới thiệu

`myls` là chương trình dòng lệnh viết bằng ngôn ngữ C, mô phỏng các chức năng của lệnh `ls` trên hệ điều hành NetBSD.

Chương trình sử dụng các API hệ thống như `opendir()`, `readdir()`, `lstat()`, `getpwuid()`, `getgrgid()` và `readlink()` để đọc và hiển thị thông tin file, thư mục.

## 2. Các option hỗ trợ

| Option | Chức năng                                       |
| --- | --- |
| `-A`   | Hiển thị file ẩn, trừ `.` và `..`               |
| `-a`   | Hiển thị tất cả file, bao gồm `.` và `..`       |
| `-c`   | Sử dụng thời gian thay đổi trạng thái file      |
| `-d`   | Hiển thị thông tin thư mục thay vì nội dung     |
| `-F`   | Thêm ký hiệu phân loại file                     |
| `-f`   | Không sắp xếp, hiển thị theo thứ tự đọc         |
| `-h`   | Hiển thị kích thước dễ đọc                      |
| `-i`   | Hiển thị inode                                  |
| `-k`   | Sử dụng đơn vị block 1 KiB                      |
| `-l`   | Hiển thị thông tin chi tiết                     |
| `-n`   | Hiển thị UID và GID bằng số                     |
| `-q`   | Thay ký tự không in được bằng `?`               |
| `-R`   | Liệt kê đệ quy các thư mục con                  |
| `-r`   | Đảo ngược thứ tự sắp xếp                        |
| `-S`   | Sắp xếp theo kích thước giảm dần                |
| `-s`   | Hiển thị số block                               |
| `-t`   | Sắp xếp theo thời gian                          |
| `-u`   | Sử dụng thời gian truy cập                      |
| `-w`   | Hiển thị nguyên bản ký tự không in được         |

## 3. Cấu trúc dự án

```text
VoQuocDat_24IT333_midterm/
├── main.c
├── ls.c
├── ls.h
├── fileinfo.c
├── fileinfo.h
├── Makefile
├── README.md
└── .gitignore
```

## 4. Hướng dẫn biên dịch

Yêu cầu hệ điều hành NetBSD và trình biên dịch C.

```bash
make
```

Chạy chương trình:

```bash
./myls
./myls -la
./myls -laF
./myls -lSh
./myls -R
```

Xóa file biên dịch:

```bash
make clean
```

## 5. Ví dụ sử dụng

```bash
./myls -a
./myls -A
./myls -l
./myls -lh
./myls -n
./myls -i
./myls -sk
./myls -F
./myls -lt
./myls -ltri
./myls -R
./myls -laF
```

## 6. Kiểm thử

Chương trình được biên dịch bằng:

```bash
cc -Wall -Wextra -std=c11 -D_NETBSD_SOURCE
```

Các kiểm thử cơ bản đã thực hiện gồm:

- Liệt kê file và thư mục.
- Hiển thị thông tin chi tiết.
- Hiển thị file ẩn.
- Sắp xếp theo tên, thời gian, kích thước.
- Hiển thị inode, block, UID/GID.
- Phân loại file thực thi và thư mục.
- Kiểm thử ký tự điều khiển với `-q` và `-w`.
- Biên dịch tự động bằng Makefile.

## 7. Hạn chế

Chương trình đã triển khai 19 option ở mức cơ bản, nhưng chưa bảo đảm tương thích hoàn toàn với mọi trường hợp đặc biệt của lệnh `ls` gốc trên NetBSD.

## 8. Kết luận

Đồ án giúp thực hành lập trình hệ thống bằng C, thao tác với file và thư mục, sử dụng API POSIX, xử lý tham số dòng lệnh và tổ chức mã nguồn theo nhiều module.
