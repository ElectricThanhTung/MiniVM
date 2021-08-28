<a id="vietnamese"></a>
***[[English below]](#english)***
## Giới thiệu về project này:
+ **Tổng quát:**
  - MiniVM là một máy ảo java nhỏ gọn có thể được triển khai và thực hiện trên các vi điều khiển 32 bit với tài nguyên hạn hẹp về phần cứng
+ **Các tính năng được hỗ trợ ở phiên bản hiện tại:**
  - Hỗ trợ 201/202 (99.5%) trên tổng số các tệp lệnh (instruction) của java.
  - Hỗ trợ trình thu gom rác với các tính năng: giải phóng các vùng nhớ/đối tượng không còn được sử dụng, Tổ chức lại bộ nhớ (dồn bộ nhớ) để chống phân mảnh bộ nhớ.
  - Hỗ trợ xử lý ngoại lệ (exception).
  - Các class được hỗ trợ trong sdk (bạn hoàn toàn có thể tự thêm các class khác cho sdk):
    - ***java.io.PrintStream***
    - ***java.lang.System***
    - ***java.lang.Object***
    - ***java.lang.String***
	- ***java.lang.Throwable***
    - ***java.lang.StringBuilder***
    - ***java.lang.StringBuffer***
    
## Cấu trúc thư mục trong project này:
|Thư mục|Mô tả chi tiết|
|:------|-----------|
|**mvm**|Thư mục này chứa source code của máy ảo java MiniVM. Project trong thư mục được viết bàng ***C/C++***, Được tạo và có thể mở bằng ***Visual Studio***.|
|**mvmtool**|Thư mục này chứa source code và chương trình thực thi của mvmtool. Project trong thư mục này được viết bằng ***C/C++***, Được tạo và có thể được mở bằng ***Visual Studio***.|
|**mvmsdk**|Thư mục này chứa source code (java) các thư viện cơ bản và cần thiết nhất để chương trình của bạn có thể sử dụng và thực thi được. Project trong thư mục này được viết bằng ***Java***, Được tạo và có thể được mở bằng ***Eclipse IDE for Java Deverlopers***.|
|**javademo**|Thư mục này chứa java code demo có thể được thực thi bởi máy ảo MiniVM. Các project trong thư mục này được tạo và có thể được mở bằng ***Eclipse IDE for Java Deverlopers***.|

## Giới thiệu về mvmtool:
+ **Tổng quan:**
  - *mvmtool* là một công cụ dùng để đóng gói chương trình java của bạn (gồm tất cả các class file trong chương trình) thành một file binary duy nhất để có thể thực thi trên máy ảo MiniVM và tính toán trước các tham số giúp giảm tải cho máy ảo trong quá trình thực thi chương trình từ đó cải thiện hiệu năng cho máy ảo.
+ **Hướng dẫn sử dụng:**<a id="guide-vn"></a>
  - ***Để sử dụng mvmtool bạn có thể làm theo 1 trong 2 cách sau:***
    - **Cách 1:** Copy file thực thi của *mvmtool* (file *mvmtool.exe* trong thư mục con ***Debug*** hoặc ***Release*** của thư mục ***mvmtool***) vào trong project java của bạn sau đó kích đúp để khởi chạy ứng dụng tại thư mục mà bạn vừa copy. Việc còn lại mvmtool sẽ quét tất các các file *.class* chứa trong project java của bạn và đóng gói thành một file binary duy nhất với tên mặc định là MiniPackage.mpk.
	- **Cách 2:** Thêm mvmtool vào biến môi trường trên máy tính của bạn sau đó sử dụng một của sổ dòng lệnh bất kỳ tại thư mục chứa project java của bạn và gõ:
	  ```sh
	  > mvmtool <tên chương trình>
	  ```
	  Trong đó *<tên chương trình>* là tên file mà bạn muốn tạo khi quá trình đóng gói hoàn tất.
	  
	  ***VD:***
	  ```sh
	  > cd helloworld
	  > mvmtool helloworld.mpk
	  ```
	  Sau khi nhập xong các dòng lệnh trong ví dụ trên, *mvmtool* sẽ tìm tất cả các file *.class* trong thư mục project của bạn (ví dụ ở đây đang là thư mục *helloworld*) sau đó đóng gói và tạo ra một file *helloworld.mpk*.
## Chạy chương trình java của bạn:
+ ***Lưu ý***:
  > Đây là hướng dẫn để thực thi chương trình java của bạn với MiniVN được triển khai như một ứng dụng windows trong project này. Nếu MiniVM được triển khai trên một nền tảng khác ví dụ như một vi điều khiển 32 bit thì cách để thực thi chương trình java của bạn có thể sẽ khác hoàn toàn với hướng dẫn này. Cách thức để thực hiện đó sẽ phụ thuộc hoàn toàn vào cách triển khai MiniVM của bạn.
+ Để thực thi được trương trình java của bạn đầu tiên bạn cần phải tạo gói *minipackage (.mpk)* của bạn bằng *mvmtoool* như [hướng dẫn ở trên](#guide-vn).
+ Sau khi đã đóng gói file *minipackage (.mpk)*, bạn có thể mở file này bằng phần mềm ***mvm.exe*** có đường dẫn trong thư mục con ***Debug*** hoặc ***Release*** của thư mục ***mvm***. Bạn có thể làm điều này bằng 1 trong 2 cách sau:
  - **Cách 1:** Kéo thả file *minipackage (.mpk)* của bạn vào file mvm.exe. Ngay sau đó chương trình của bạn sẽ được thực thi.
  - **Cách 2:** Chuột phải vào file *minipackage (.mpk)* của bạn và chon "***Open with***" sau đó tìm và dẫn đến file ***mvm.exe*** trên ổ đĩa của bạn (Nên tick "***Always use this app to open .mpk files***" để tiện sử dụng cho các lần sau).
---
<a id="english"></a>
***[[Tiếng việt ở trên]](#vietnamese)***

## About this project:
+ **Overview:**
  - MiniVM is a small java virtual machine that can run on 32 bit MCUs with limmited resource
+ **Current version's features:**
  - Support 201/202 (99.5%) of java's instructions.
  - Support garbage collector with features: Release memory/objects not used, memory reorganization to avoid fragmentation memory.
  - Exception handling support.
  - Supported Classes in this version (you can add other classes for sdk):
    - ***java.io.PrintStream***
    - ***java.lang.System***
    - ***java.lang.Object***
    - ***java.lang.String***
	- ***java.lang.Throwable***
    - ***java.lang.StringBuilder***
	- ***java.lang.StringBuffer***
    
## Folder structure in this project:
|Folder name|Description|
|:------|-----------|
|**mvm**|This folder contains the source code MiniVM. Project in this folder is written in ***C/C++*** language, created/open by ***Visual Studio***.|
|**mvmtool**|This folder contains the source code and executable program of MiniVM. Project in this folder is written in ***C/C++*** language, created/open by ***Visual Studio***.|
|**mvmsdk**|This folder contains source code of the base classes/library and necessary of the sdk. Project in this folder is written in ***Java*** language, created/open by ***Eclipse IDE for Deverlopers***.|
|**javademo**|This folder contains java codes demo will be executed by MiniVM, Projects in this folder created/open by ***Eclipse IDE for Deverlopers***.|

## About mvmtool:
+ **Overview:**
  - *mvmtool* is a tool used to package your java program (include all classes file in your java program) into a single binary file can run on MiniVM and pre-compute parameters to reduce the load on the virtual machine during program execution, thereby improving the performance of the virtual machine.
+ **User manual:**<a id="guide-eg"></a>
  - ***To use mvmtool you can do it in 1 of 2 ways:***
    - **Way 1:** Copy *mvmtool's* excutable file (*mvmtool.exe* file in subfolder ***Debug*** or***Release*** of ***mvmtool*** folder) into your java project and then double click it to run this program. mvmtool will scan and find all *.class* files contained in your java project and pack into a single binary file with default name is MiniPackage.mpk
	- **way 2:** Add mvmtool to the environment variable on your computer and then use any command line at the directory where your java project is located and type:
	  ```sh
	  > mvmtool <package name>
	  ```
	  *\<package name>* is the file name that you want to create when the packaging process is completed.
	  
	  ***Ex:***
	  ```sh
	  > cd helloworld
	  > mvmtool helloworld.mpk
	  ```
	  After enter commands above done, *mvmtool* will find all *.class* files in your folder project (in the example is folder helloworld) then pack and create *helloworld.mpk* file.
## How to run your java program with MiniVM:
+ ***Note***:
  > This is guide to run your java program with MiniVM implemented as a windows program in this project. If MiniVM implemented on another system (ex a 32 bits MCU), maybe way to run your java program will different from this guide. It depends on your implementation.
+ To run your java program, you have to create your *minipackage (.mpk)* package first by *mvmtoool*   
as [guide above](#guide-eg)
+ After packing the file *minipackage (.mpk)*, you can open this file by ***mvm.exe*** program. This program has path in subfolder ***Debug*** or ***Release*** of the ***mvm*** folder. You can do it in 1 of 2 ways:
  - **Way 1:** Drag and drop your *minipackage (.mpk)* file into mvm.exe file. After that your java program will be executed.
  - **Way 2:** Right click on your *minipackage (.mpk)* file and select "***Open with***" then find ***mvm.exe*** file on your disk and click "***OK***" (should tick "***Always use this app to open .mpk files***" to remember the selection for  next time open)