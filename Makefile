OBJECTS = icons.o wombatjmp.o managetags.o aboutbox.o viewer.o cfb.o jmp.o lnk.o

ICONS = resources/folder-open.png resources/help-contents.png resources/managetags.png resources/paperairplane2.png resources/reportpreview1.png resources/wombat_32.ico resources/bookmark.png resources/bookmark-new.png resources/bookmark-rem.png resources/aboutwombat.png resources/mail.png

# Generate header file and source file containing declarations and data arrays
icons: $(ICONS) icons.h
	reswrap --header -o icons.h $(ICONS) --source -i icons.h -o icons.cpp $(ICONS)

# Link it all together

wombatjmp: $(OBJECTS)
	g++ -O3 -o wombatjmp $(OBJECTS) -lX11 -lXext -lXft -lfontconfig -lfreetype -lXcursor -lXrender -lXrandr -lXfixes -lXi -lGLU -lGL -ldl -lpthread -ljpeg -lrt -lpng -ltiff -lz -lbz2 -lcrypto -lgsasl -lgnutls -licuuc libFOX-1.7.a

wombatjmp.o: wombatjmp.cpp icons.h managetags.cpp aboutbox.cpp viewer.cpp cfb.cpp jmp.cpp lnk.cpp
