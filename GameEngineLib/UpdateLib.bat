xcopy			/y		..\CoreLib\Bin\Debug\CoreLib.lib			.\Lib\Debug\
xcopy			/y		..\CoreLib\Bin\Release\CoreLib.lib			.\Lib\Release\

xcopy			/y		..\RenderLib\Bin\Debug\RenderLib.lib		.\Lib\Debug\
xcopy			/y		..\RenderLib\Bin\Release\RenderLib.lib		.\Lib\Release\

xcopy			/y		..\PhysicsLib\Bin\Debug\PhysicsLib.lib			.\Lib\Debug\
xcopy			/y		..\PhysicsLib\Bin\Release\PhysicsLib.lib		.\Lib\Release\

xcopy			/y		..\CoreLib\*.h									.\Inc\
xcopy			/y		..\RenderLib\*.h								.\Inc\
xcopy			/y		..\PhysicsLib\*.h								.\Inc\

xcopy			/y/s	..\RenderLib\Lib\Debug\*.*					.\Lib\Debug\
xcopy			/y/s	..\RenderLib\Lib\Release\*.*				.\Lib\Release\

xcopy			/y/s	..\CoreLib\Inc\*.*							.\Inc\
xcopy			/y/s	..\RenderLib\Inc\*.*						.\Inc\
