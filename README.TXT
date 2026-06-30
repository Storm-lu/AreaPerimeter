========================================================================
       MICROSOFT FOUNDATION CLASS LIBRARY : DSplit
========================================================================


AppWizard has created this DSplit application for you.  This application
not only demonstrates the basics of using the Microsoft Foundation classes
but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your DSplit application.

DSplit.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CDSplitApp application class.

DSplit.cpp
    This is the main application source file that contains the application
    class CDSplitApp.

DSplit.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
	Developer Studio.

res\DSplit.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file DSplit.rc.

res\DSplit.rc2
    This file contains resources that are not edited by Microsoft 
	Developer Studio.  You should place all resources not
	editable by the resource editor in this file.

DSplit.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.

res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the
    CMainFrame class.  Edit this toolbar bitmap along with the
    array in MainFrm.cpp to add more toolbar buttons.

/////////////////////////////////////////////////////////////////////////////

AppWizard creates one document type and one view:

DSplitDoc.h, DSplitDoc.cpp - the document
    These files contain your CDSplitDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CDSplitDoc::Serialize).

DSplitView.h, DSplitView.cpp - the view of the document
    These files contain your CDSplitView class.
    CDSplitView objects are used to view CDSplitDoc objects.



/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named DSplit.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Developer Studio reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, and your application is 
in a language other than the operating system's current language, you
will need to copy the corresponding localized resources MFC40XXX.DLL
from the Microsoft Visual C++ CD-ROM onto the system or system32 directory,
and rename it to be MFCLOC.DLL.  ("XXX" stands for the language abbreviation.
For example, MFC40DEU.DLL contains resources translated to German.)  If you
don't do this, some of the UI elements of your application will remain in the
language of the operating system.

/////////////////////////////////////////////////////////////////////////////


README.TXT

微软基础类库：DSplit 项目

AppWizard 已为您创建了此 DSplit 应用程序。该应用程序不仅演示了使用微软基础类库的基本知识，同时也是您编写自己应用程序的起点。

本文件概述了构成 DSplit 应用程序的各个文件的内容。

DSplit.h
这是应用程序的主头文件。它包含了其他特定于项目的头文件（包括 Resource.h），并声明了 CDSplitApp 应用程序类。

DSplit.cpp
这是包含 CDSplitApp 应用程序类的主要应用程序源文件。

DSplit.rc
这是程序使用的所有 Microsoft Windows 资源的清单。它包含在 RES 子目录中存储的图标、位图和光标。该文件可直接在 Microsoft Developer Studio 中进行编辑。

res\DSplit.ico
这是用作应用程序图标的图标文件。该图标由主资源文件 DSplit.rc 包含。

res\DSplit.rc2
此文件包含不由 Microsoft Developer Studio 编辑的资源。您应将所有资源编辑器无法编辑的资源放在此文件中。

DSplit.clw
此文件包含 ClassWizard 用于编辑现有类或添加新类的信息。ClassWizard 还使用此文件来存储创建和编辑消息映射、对话框数据映射以及创建原型成员函数所需的信息。

/////////////////////////////////////////////////////////////////////////////

关于主框架窗口：

MainFrm.h, MainFrm.cpp
这些文件包含框架类 CMainFrame，该类派生自 CFrameWnd，并控制所有 SDI（单文档界面）框架功能。

res\Toolbar.bmp
此位图文件用于创建工具栏的平铺图像。初始的工具栏和状态栏是在 CMainFrame 类中构建的。编辑此工具栏位图以及 MainFrm.cpp 中的数组，以添加更多工具栏按钮。

/////////////////////////////////////////////////////////////////////////////

AppWizard 创建一个文档类型和一个视图：

DSplitDoc.h, DSplitDoc.cpp - 文档
这些文件包含您的 CDSplitDoc 类。编辑这些文件以添加您的专用文档数据，并实现文件保存和加载功能（通过 CDSplitDoc::Serialize）。

DSplitView.h, DSplitView.cpp - 文档视图
这些文件包含您的 CDSplitView 类。CDSplitView 对象用于查看 CDSplitDoc 对象。

/////////////////////////////////////////////////////////////////////////////

其他标准文件：

StdAfx.h, StdAfx.cpp
这些文件用于构建名为 DSplit.pch 的预编译头文件 (PCH) 和名为 StdAfx.obj 的预编译类型文件。

Resource.h
这是标准头文件，定义了新的资源 ID。Microsoft Developer Studio 会读取并更新此文件。

/////////////////////////////////////////////////////////////////////////////

其他注意事项：

AppWizard 使用 “TODO:” 来标记您应该添加或自定义的源代码部分。

如果您的应用程序在共享 DLL 中使用 MFC，并且您的应用程序语言与操作系统当前语言不同，您需要将对应的本地化资源 MFC40XXX.DLL 从 Microsoft Visual C++ 安装光盘复制到系统的 system 或 system32 目录下，并将其重命名为 MFCLOC.DLL。（“XXX”代表语言缩写。例如，MFC40DEU.DLL 包含翻译成德语的资源。）如果您不这样做，您的应用程序中的某些 UI 元素将保留为操作系统的语言。

/////////////////////////////////////////////////////////////////////////////
