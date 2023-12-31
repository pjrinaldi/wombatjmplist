#include "lnk.h"

void ParseShortcut(FileItem* curfileitem, uint8_t* tmpbuf, std::string* filecontents)
{
    /*
    if(!inmemory)
    {
        std::ifstream file(tmpfilestr.c_str(),  std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        tmpbuf = new uint8_t[size];
        file.read((char*)tmpbuf, size);
    }
    */
    uint32_t flags = 0;
    uint32_t attributes = 0;
    uint64_t created = 0;
    uint64_t modified = 0;
    uint64_t accessed = 0;
    uint32_t filesize = 0;
    uint16_t shellstructurelength = 0;
    uint32_t voloffset = 0;
    uint32_t volstructurelength = 0;
    uint32_t voltype = 0;
    uint32_t volserial = 0;
    uint32_t volnameoffset = 0;
    uint32_t basepathoffset = 0;
    uint32_t networkvoloffset = 0;
    uint32_t remainingpathoffset = 0;
    uint32_t totalstructurelength = 0;
    std::string volnamestr = "";
    std::string basepathstr = "";
    std::string descstring = "";
    std::string relpathstr = "";
    std::string workingdirectory  = "";
    std::string commandstring = "";
    std::string iconstring = "";
    int curoffset = 0;

    std::string titlestring = "LNK File Analysis for " + curfileitem->name + " (" + std::to_string(curfileitem->gid) + ")";
    filecontents->clear();
    filecontents->append(titlestring + "\n");
    for(int i=0; i < titlestring.size(); i++)
        filecontents->append("-");
    filecontents->append("\n\n");

    ReadInteger(tmpbuf, 20, &flags);
    //if(flags & 0x80)
    //    std::cout << "data strings are unicode rather than ascii." << std::endl;
    //std::cout << "flags: " << std::hex << flags << std::dec << std::endl;
    ReadInteger(tmpbuf, 24, &attributes);
    filecontents->append("File Attributes\t\t| ");
    if(attributes & 0x01)
        filecontents->append("Read Only,");
    if(attributes & 0x02)
        filecontents->append("Hidden,");
    if(attributes & 0x04)
        filecontents->append("System,");
    if(attributes & 0x10)
        filecontents->append("Directory,");
    if(attributes & 0x20)
        filecontents->append("Archived,");
    if(attributes & 0x80)
        filecontents->append("Normal");
    if(attributes & 0x100)
        filecontents->append("Temporary,");
    if(attributes & 0x200)
        filecontents->append("Sparse,");
    if(attributes & 0x400)
        filecontents->append("Reparse Point||Symbolic Link,");
    if(attributes & 0x800)
        filecontents->append("Compressed,");
    if(attributes & 0x1000)
        filecontents->append("Offline,");
    if(attributes & 2000)
        filecontents->append("Not Indexed,");
    if(attributes & 4000)
        filecontents->append("Encrypted,");
    if(attributes & 10000)
        filecontents->append("Virtual");
    filecontents->append("\n");
    //std::cout << "attributes: " << std::hex << attributes << std::dec << std::endl;
    ReadInteger(tmpbuf, 28, &created);
    //std::cout << "Created: " << ConvertWindowsTimeToUnixTimeUTC(created) << std::endl;
    ReadInteger(tmpbuf, 36, &modified);
    ReadInteger(tmpbuf, 44, &accessed);
    ReadInteger(tmpbuf, 52, &filesize);
    filecontents->append("Created\t\t\t| " + ConvertWindowsTimeToUnixTimeUTC(created) + "\n");
    filecontents->append("Modified\t\t| " + ConvertWindowsTimeToUnixTimeUTC(modified) + "\n");
    filecontents->append("Accessed\t\t| " + ConvertWindowsTimeToUnixTimeUTC(accessed) + "\n");
    filecontents->append("File Size\t\t| " + ReturnFormattingSize(filesize) + " bytes\n\n");
    if(flags & 0x01) // SHELL ITEM ID LIST IS PRESENT
    {
        // PARSE SHELL ID LIST HERE
        ReadInteger(tmpbuf, 76, &shellstructurelength);
        libfwsi_item_list_t* itemlist = NULL;
        libfwsi_error_t* itemerror = NULL;
        int ret = libfwsi_item_list_initialize(&itemlist, &itemerror);
        uint8_t* itemstream = new uint8_t[shellstructurelength];
        itemstream = substr(tmpbuf, 78, shellstructurelength);
        ret = libfwsi_item_list_copy_from_byte_stream(itemlist, itemstream, shellstructurelength, LIBFWSI_CODEPAGE_ASCII, &itemerror);
        int itemlistcount = 0;
        ret = libfwsi_item_list_get_number_of_items(itemlist, &itemlistcount, &itemerror);
        //std::cout << "number of list items: " << itemlistcount << std::endl;
        for(int i=0; i < itemlistcount; i++) // parse each shell item
        {
            filecontents->append("Shell Item\t\t| " + std::to_string(i+1) + "\n");
            filecontents->append("\tItem Type\t| ");
            libfwsi_item_t* curitem;
            ret = libfwsi_item_initialize(&curitem, &itemerror);
            ret = libfwsi_item_list_get_item(itemlist, i, &curitem, &itemerror);
            int itemtype = 0;
            ret = libfwsi_item_get_type(curitem, &itemtype, &itemerror);
            //std::cout << "item type: " << itemtype << std::endl;
            uint8_t classtype = 0;
            ret = libfwsi_item_get_class_type(curitem, &classtype, &itemerror);
            //std::cout << "class type: " << (uint)classtype << std::endl;
            if(itemtype == 0)
                filecontents->append("Unknown");
            else if(itemtype == 1)
                filecontents->append("CDBurn");
            else if(itemtype == 2)
                filecontents->append("Compressed Folder");
            else if(itemtype == 3)
                filecontents->append("Control Panel");
            else if(itemtype == 4)
                filecontents->append("Control Panel Category");
            else if(itemtype == 5)
                filecontents->append("Control Panel CPL File");
            else if(itemtype == 6)
                filecontents->append("Delegate");
            else if(itemtype == 7)
                filecontents->append("File Entry");
            else if(itemtype == 8)
                filecontents->append("Game Folder");
            else if(itemtype == 9)
                filecontents->append("MTP File Entry");
            else if(itemtype == 10)
                filecontents->append("MTP Volume");
            else if(itemtype == 11)
                filecontents->append("Network Location");
            else if(itemtype == 12)
                filecontents->append("Root Folder");
            else if(itemtype == 13)
                filecontents->append("URI");
            else if(itemtype == 14)
                filecontents->append("URI Sub Values");
            else if(itemtype == 15)
                filecontents->append("User's Property View");
            else if(itemtype == 16)
                filecontents->append("Volume");
            filecontents->append("\n");
            filecontents->append("\tClass Type\t| ");
            if((uint)classtype < 32)
            {
                filecontents->append("Root Folder\n");
                uint8_t* rootguid = new uint8_t[16];
                libfwsi_root_folder_get_shell_folder_identifier(curitem, rootguid, 16, &itemerror);
                filecontents->append("\tIdentifier\t| " + ReturnFormattedGuid(rootguid) + "\n");
                filecontents->append("\tFolder Nmae\t| " + std::string(libfwsi_shell_folder_identifier_get_name(rootguid)) + "\n");
                delete[] rootguid;
            }
            else if((uint)classtype < 48 && (uint)classtype >= 32)
            {
                filecontents->append("Volume\n");
                size_t volnamesize = libfwsi_volume_get_utf8_name_size(curitem, &volnamesize, &itemerror);
                //std::cout << "vol name size: " << volnamesize << std::endl;
                uint8_t* volname = new uint8_t[volnamesize+1];
                ret = libfwsi_volume_get_utf8_name(curitem, volname, volnamesize, &itemerror);
                volname[volnamesize] = 0;
                filecontents->append("\tVolume Name\t| " + std::string((char*)volname) + ":\\ \n");
                delete[] volname;
            }
            else if((uint)classtype < 64 && (uint)classtype >= 48)
            {
                filecontents->append("File Entry\n");
                size_t filenamesize = 0;
                ret = libfwsi_file_entry_get_utf8_name_size(curitem, &filenamesize, &itemerror);
                uint8_t* fname = new uint8_t[filenamesize+1];
                ret = libfwsi_file_entry_get_utf8_name(curitem, fname, filenamesize, &itemerror);
                fname[filenamesize] = 0;
                filecontents->append("\tName\t\t| " + std::string((char*)fname) + "\n");
                delete[] fname;
                uint32_t fatdatetime = 0;
                ret = libfwsi_file_entry_get_modification_time(curitem, &fatdatetime, &itemerror);
                uint8_t* fdt = (uint8_t*)&fatdatetime;
                uint16_t fatdate = (uint16_t)fdt[0] | (uint16_t)fdt[1] << 8;
                uint16_t fattime = (uint16_t)fdt[2] | (uint16_t)fdt[3] << 8;
                filecontents->append("\tModified Time\t| " + ConvertDosTimeToHuman(&fatdate, &fattime) + " UTC\n");
                uint32_t attrflags = 0;
                ret = libfwsi_file_entry_get_file_attribute_flags(curitem, &attrflags, &itemerror);
                filecontents->append("\tFile Attributes\t| ");
                if(attrflags & 0x01)
                    filecontents->append("Read Only,");
                if(attrflags & 0x02)
                    filecontents->append("Hidden,");
                if(attrflags & 0x04)
                    filecontents->append("System,");
                if(attrflags & 0x10)
                    filecontents->append("Directory,");
                if(attrflags & 0x20)
                    filecontents->append("Archive,");
                if(attrflags & 0x40)
                    filecontents->append("Device,");
                if(attrflags & 0x80)
                    filecontents->append("Normal,");
                if(attrflags & 0x100)
                    filecontents->append("Temporary,");
                if(attrflags & 0x200)
                    filecontents->append("Sparse,");
                if(attrflags & 0x400)
                    filecontents->append("Reparse Point,");
                if(attrflags & 0x800)
                    filecontents->append("Commpressed,");
                if(attrflags & 0x1000)
                    filecontents->append("Offline,");
                if(attrflags & 0x2000)
                    filecontents->append("Not Indexed,");
                if(attrflags & 0x4000)
                    filecontents->append("Encrypted,");
                if(attrflags & 0x10000)
                    filecontents->append("Virtual");
                filecontents->append("\n");
            }
            else if((uint)classtype < 80 && (uint)classtype >= 64)
            {
                filecontents->append("Network Location\n");
                size_t locsize = 0;
                ret = libfwsi_network_location_get_utf8_location_size(curitem, &locsize, &itemerror);
                uint8_t* locname = new uint8_t[locsize+1];
                ret = libfwsi_network_location_get_utf8_location(curitem, locname, locsize, &itemerror);
                locname[locsize] = 0;
                filecontents->append("\tLocation\t| " + std::string((char*)locname) + "\n");
                delete[] locname;
                size_t descsize = 0;
                ret = libfwsi_network_location_get_utf8_description_size(curitem, &descsize, &itemerror);
                uint8_t* descname = new uint8_t[descsize + 1];
                ret = libfwsi_network_location_get_utf8_description(curitem, descname, descsize, &itemerror);
                descname[descsize] = 0;
                filecontents->append("\tDescription\t| " + std::string((char*)descname) + "\n");
                delete[] descname;
                size_t comsize = 0;
                ret = libfwsi_network_location_get_utf8_comments_size(curitem, &comsize, &itemerror);
                uint8_t* comname = new uint8_t[comsize+1];
                ret = libfwsi_network_location_get_utf8_comments(curitem, comname, comsize, &itemerror);
                comname[comsize] = 0;
                filecontents->append("\tComments\t| " + std::string((char*)comname) + "\n");
                delete[] comname;
            }
            else if((uint)classtype < 96 && (uint)classtype >= 80)
            {
                filecontents->append("URI\n");
            }
            
            int extblkcount = 0;
            ret = libfwsi_item_get_number_of_extension_blocks(curitem, &extblkcount, &itemerror);
            //std::cout << "extblkcnt: " << extblkcount << std::endl;
            for(int j=0; j < extblkcount; j++)
            {
                libfwsi_extension_block_t* curblock = NULL;
                libfwsi_item_get_extension_block(curitem, j, &curblock, &itemerror);
                uint32_t extsig = 0;
                ret = libfwsi_extension_block_get_signature(curblock, &extsig, &itemerror);
                std::stringstream extstream;
                extstream << std::hex << extsig;
                if(extsig == 0xbeef0004)
                {
                    filecontents->append("Extension Block\t\t| " + std::to_string(j+1) + "\n");
                    filecontents->append("\tSignature\t| 0x" + extstream.str() + " File Entry Extension\n");
                }
                uint32_t creationtime = 0;
                ret = libfwsi_file_entry_extension_get_creation_time(curblock, &creationtime, &itemerror);
                uint8_t* cdt = (uint8_t*)&creationtime;
                uint16_t cdate = (uint16_t)cdt[0] | (uint16_t)cdt[1] << 8;
                uint16_t ctime = (uint16_t)cdt[2] | (uint16_t)cdt[3] << 8;
                filecontents->append("\tCreated Time\t| " + ConvertDosTimeToHuman(&cdate, &ctime) + " UTC\n");
                uint32_t accesstime = 0;
                ret = libfwsi_file_entry_extension_get_access_time(curblock, &accesstime, &itemerror);
                uint8_t* adt = (uint8_t*)&accesstime;
                uint16_t adate = (uint16_t)adt[0] | (uint16_t)adt[1] << 8;
                uint16_t atime = (uint16_t)adt[2] | (uint16_t)adt[3] << 8;
                filecontents->append("\tAccessed Time\t| " + ConvertDosTimeToHuman(&adate, &atime) + " UTC\n");
                size_t lnamesize = 0;
                ret = libfwsi_file_entry_extension_get_utf8_long_name_size(curblock, &lnamesize, &itemerror);
                uint8_t* lname = new uint8_t[lnamesize+1];
                ret = libfwsi_file_entry_extension_get_utf8_long_name(curblock, lname, lnamesize, &itemerror);
                filecontents->append("\tLong Name\t| " + std::string((char*)lname) + "\n");
                delete[] lname;
                libfwsi_extension_block_free(&curblock, &itemerror);
            }
            filecontents->append("\n");
            ret = libfwsi_item_free(&curitem, &itemerror);
        }

        libfwsi_item_list_free(&itemlist, &itemerror);
        libfwsi_error_free(&itemerror);
        delete[] itemstream;
        //std::cout << "shell structure length: " << shellstructurelength << std::endl;
        curoffset = 76 + shellstructurelength + 2;
    }
    //std::cout << "curoffset: " << curoffset << std::endl;
    if(flags & 0x02)
    {
        // GET structure offsets
        ReadInteger(tmpbuf, curoffset, &totalstructurelength);
        //std::cout << "total structure length: " << totalstructurelength << std::endl;
        uint32_t nextoffset = 0;
        ReadInteger(tmpbuf, curoffset + 4, &nextoffset);
        //std::cout << "next offset after this: " << nextoffset << std::endl;
        uint32_t fileflags = 0;
        ReadInteger(tmpbuf, curoffset + 8, &fileflags);
        //std::cout << "file flags: " << fileflags << std::endl;
        std::bitset<8> volflagbits(fileflags);
        //std::cout  << "volflagbits: " << volflagbits << std::endl;
        if(volflagbits[0] == 1)
        {
            ReadInteger(tmpbuf, curoffset + 12, &voloffset);
            ReadInteger(tmpbuf, curoffset + 16, &basepathoffset);
            //std::cout << "voloffset: " << voloffset << std::endl;
            //std::cout << " basepathoffset: " << basepathoffset << std::endl;
            ReadInteger(tmpbuf, curoffset + voloffset, &volstructurelength);
            ReadInteger(tmpbuf, curoffset + voloffset + 4, &voltype);
            filecontents->append("Volume Type\t\t| ");
            if(voltype == 0)
                filecontents->append("Unknown (0)");
            else if(voltype == 1)
                filecontents->append("No Root Directory (1)");
            else if(voltype == 2)
                filecontents->append("Removable (2)");
            else if(voltype == 3)
                filecontents->append("Fixed (3)");
            else if(voltype == 4)
                filecontents->append("Remote (4)");
            else if(voltype == 5)
                filecontents->append("Optical Disc (5)");
            else if(voltype == 6)
                filecontents->append("RAM Drive (6)");
            filecontents->append("\n");
            //std::cout << "voltype: " << voltype << std::endl;
            ReadInteger(tmpbuf, curoffset + voloffset + 8, &volserial);
            std::stringstream serialstream;
            serialstream << std::hex << volserial;
            filecontents->append("Volume Serial Number\t| 0x" + serialstream.str() + "\n");
            //std::cout << "vol serial: 0x" << std::hex << volserial << std::dec << std::endl;
            ReadInteger(tmpbuf, curoffset + voloffset + 12, &volnameoffset);
            //std::cout << "name length: " << volstructurelength << " " << volnameoffset << std::endl;
            //std::cout << "vol name length: " << volstructurelength - volnameoffset << std::endl;
            uint8_t* volname = new uint8_t[volstructurelength - volnameoffset +1];
            volname = substr(tmpbuf, curoffset + voloffset + volnameoffset, volstructurelength - volnameoffset);
            volname[volstructurelength - volnameoffset] = 0;
            //std::cout << "volname: " << (char*)volname << std::endl;
            volnamestr = std::string((char*)volname);
            filecontents->append("Volume Label\t\t| " + volnamestr + "\n");
            delete[] volname;
            //std::cout << "basepath global offset: " << curoffset + basepathoffset << std::endl;
            //std::cout << "next offset: " << curoffset + voloffset + volnameoffset + volstructurelength - volnameoffset + 1 << std::endl;
            //std::cout << "basepath length: " << totalstructurelength - basepathoffset - 1 << std::endl;
            uint8_t* basepathname = new uint8_t[totalstructurelength - basepathoffset];
            basepathname = substr(tmpbuf, curoffset + basepathoffset, totalstructurelength - basepathoffset - 1);
            basepathname[totalstructurelength - basepathoffset] = 0;
            basepathstr = std::string((char*)basepathname);
            filecontents->append("Local Path\t\t| " + basepathstr + "\n");
            delete[] basepathname;
            //std::cout << "base path name: " << (char*)basepathname << std::endl;
        }
        if(volflagbits[1] == 1)
        {
            ReadInteger(tmpbuf, curoffset + 20, &networkvoloffset);
            // PARSE NETWORK VOLUME STRUCTURE HERE
            //std::cout << "networkvoloffset: " << networkvoloffset << std::endl;
        }
        ReadInteger(tmpbuf, curoffset + 24, &remainingpathoffset);
        // PARSE REMAINING/FINAL PATH HERE
        //std::cout << "remainingpathoffset: " << remainingpathoffset << std::endl;
        //std::cout << "file or directory" << std::endl;
    }
    //std::cout << "on to different strings: " << curoffset + totalstructurelength << std::endl;
    curoffset = curoffset + totalstructurelength;
    if(flags & 0x04)
    {
        //std::cout << "has a descriptive string" << std::endl;
        uint16_t desclength = 0;
        ReadInteger(tmpbuf, curoffset, &desclength);
        //std::cout << "desclength: " << desclength << std::endl;
        if(flags & 0x80) // utf-16 unicode
        {
            for(int i=0; i < desclength; i++)
            {
                uint16_t singleletter = 0;
                ReadInteger(tmpbuf, curoffset + 2 + i*2, &singleletter);
                descstring += (char)singleletter;
            }
            curoffset = curoffset + desclength * 2 + 2;
        }
        else // ascii
        {
            uint8_t* description = new uint8_t[desclength+1];
            description = substr(tmpbuf, curoffset + 2, desclength);
            description[desclength] = 0;
            descstring = std::string((char*)description);
            //std::cout << "description: " << (char*)description << std::endl;
            delete[] description;
            curoffset = curoffset + desclength + 2;
        }
        filecontents->append("Description\t\t| " + descstring + "\n");
        //std::cout << "description: " << descstring << std::endl;
    }
    if(flags & 0x08)
    {
        //std::cout << "has a relative path string" << std::endl;
        uint16_t relpathlength = 0;
        ReadInteger(tmpbuf, curoffset, &relpathlength);
        //std::cout << "relpathlength: " << relpathlength << std::endl;
        if(flags & 0x80)
        {
            for(int i=0; i < relpathlength; i++)
            {
                uint16_t singleletter = 0;
                ReadInteger(tmpbuf, curoffset + 2 + i*2, &singleletter);
                relpathstr += (char)singleletter;
            }
            curoffset = curoffset + relpathlength * 2 + 2;
        }
        else
        {
            uint8_t* relpath = new uint8_t[relpathlength+1];
            relpath = substr(tmpbuf, curoffset + 2, relpathlength);
            relpath[relpathlength] = 0;
            relpathstr = std::string((char*)relpath);
            //std::cout << "relpath: " << relpath << std::endl;
            delete[] relpath;
            curoffset = curoffset + relpathlength + 2;
        }
        filecontents->append("Relative Path\t| " + relpathstr + "\n");
    }
    if(flags & 0x10)
    {
        //std::cout << "has a working directory" << std::endl;
        uint16_t workdirlength = 0;
        ReadInteger(tmpbuf, curoffset, &workdirlength);
        if(flags & 0x80)
        {
            for(int i=0; i < workdirlength; i++)
            {
                uint16_t singleletter = 0;
                ReadInteger(tmpbuf, curoffset + 2 + i*2, &singleletter);
                workingdirectory += (char)singleletter;
            }
            curoffset = curoffset + workdirlength * 2 + 2;
        }
        else
        {
            uint8_t* workdirchar = new uint8_t[workdirlength+1];
            workdirchar = substr(tmpbuf, curoffset + 2, workdirlength);
            workdirchar[workdirlength] = 0;
            workingdirectory = std::string((char*)workdirchar);
            delete[] workdirchar;
            curoffset = curoffset + workdirlength + 2;
        }
        filecontents->append("Working Directory\t| " + workingdirectory + "\n");
        //std::cout << "working directory: " << workingdirectory << std::endl;
    }
    if(flags & 0x20)
    {
        //std::cout << "has command line arguments" << std::endl;
        uint16_t cmdlength = 0;
        ReadInteger(tmpbuf, curoffset, &cmdlength);
        if(flags & 0x80) // UTF-16 UNICODE
        {
            for(int i=0; i < cmdlength; i++)
            {
                uint16_t singleletter = 0;
                ReadInteger(tmpbuf, curoffset + 2 + i*2, &singleletter);
                commandstring += (char)singleletter;
            }
            curoffset = curoffset + cmdlength * 2 + 2;
        }
        else // ASCII
        {
            uint8_t* cmdchar = new uint8_t[cmdlength+1];
            cmdchar = substr(tmpbuf, curoffset + 2, cmdlength);
            cmdchar[cmdlength] = 0;
            commandstring = std::string((char*)cmdchar);
            delete[] cmdchar;
            curoffset = curoffset + cmdlength + 2;
        }
        filecontents->append("Command Line\t| " + commandstring + "\n");
        //std::cout << "cmnd string: " << commandstring << std::endl;
    }
    if(flags & 0x40)
    {
        //std::cout << "has a custom icon" << std::endl;
        uint16_t iconlength = 0;
        ReadInteger(tmpbuf, curoffset, &iconlength);
        if(flags & 0x80) // UTF-16 UNICODE
        {
            for(int i=0; i < iconlength; i++)
            {
                uint16_t singleletter = 0;
                ReadInteger(tmpbuf, curoffset + 2 + i*2, &singleletter);
                iconstring += (char)singleletter;
            }
            curoffset = curoffset + iconlength * 2 + 2;
        }
        else // ASCII
        {
            uint8_t* iconchar = new uint8_t[iconlength + 1];
            iconchar = substr(tmpbuf, curoffset + 2, iconlength);
            iconchar[iconlength] = 0;
            iconstring = std::string((char*)iconchar);
            delete[] iconchar;
            curoffset = curoffset + iconlength + 2;
        }
        filecontents->append("Icon File\t\t| " + iconstring + "\n");
        //std::cout << "icon string: " << iconstring << std::endl;
    }
    filecontents->append("\n");
    //std::cout << "curoffset after strings: " << curoffset << std::endl;
    //std::cout << "flags: " << flags << std::endl;
    // PARSE EXTRA BLOCKS HERE
    // THIS WORKS, NEED TO GET DISTRIBUTED TRACKING LINK BIT TO GET MACHINE IDENTIFIER
    int k = 1;
    while(curoffset < curfileitem->size)
    {
        uint32_t blksize = 0;
        ReadInteger(tmpbuf, curoffset, &blksize);
        if(blksize == 0)
            break;
        filecontents->append("Data Block\t\t| " + std::to_string(k) + "\n");
        uint32_t blksig = 0;
        ReadInteger(tmpbuf, curoffset + 4, &blksig);
        if(blksig == 0xa0000001) // environment variables location (788 bytes)
        {
            filecontents->append("\tSignature\t| Environment Variables Location (0xa0000001)\n");
            uint8_t* evloc = new uint8_t[260];
            evloc = substr(tmpbuf, curoffset + 8, 260);
            filecontents->append("\tENV VAR Location\t| " + std::string((char*)evloc) + "\n");
            delete[] evloc;
            k++;
            curoffset = curoffset + 788;
        }
        else if(blksig == 0xa0000002) // console properties (204 bytes)
        {
            filecontents->append("\tSignature\t| Console Properties (0xa0000002)\n");
            k++;
            curoffset = curoffset + 204;
        }
        else if(blksig == 0xa0000003) // distributed link tracker properties (96 bytes)
        {
            filecontents->append("\tSignature\t| Distributed Link Tracker Properties (0xa0000003)\n");
            uint8_t* machineid = new uint8_t[16];
            machineid = substr(tmpbuf, curoffset + 16, 16);
            filecontents->append("\tMachine ID\t| " + std::string((char*)machineid) + "\n");
            delete[] machineid;
            uint8_t* dvid = new uint8_t[16];
            dvid = substr(tmpbuf, curoffset + 32, 16);
            filecontents->append("\tDroid Volume ID | " + ReturnFormattedGuid(dvid) + "\n");
            delete[] dvid;
            uint8_t* fid = new uint8_t[16];
            fid = substr(tmpbuf, curoffset + 48, 16);
            filecontents->append("\tDroid File ID\t| " + ReturnFormattedGuid(fid) + "\n");
            delete[] fid;
            uint8_t* bdvid = new uint8_t[16];
            bdvid = substr(tmpbuf, curoffset + 64, 16);
            filecontents->append("\tBirth Volume ID | " + ReturnFormattedGuid(bdvid) + "\n");
            delete[] bdvid;
            uint8_t* bfid = new uint8_t[16];
            bfid = substr(tmpbuf, curoffset + 80, 16);
            filecontents->append("\tBirth File ID\t| " + ReturnFormattedGuid(bfid) + "\n");
            delete[] bfid;
            k++;
            curoffset = curoffset + 96;
        }
        else if(blksig == 0xa0000004) // console codepage (12 bytes)
        {
            filecontents->append("\tSignature\t| Console Codepage (0xa0000004)\n");
            k++;
            curoffset = curoffset + 12;
        }
        else if(blksig == 0xa0000005) // special folder location (16 bytes)
        {
            filecontents->append("\tSignature\t| Special Folder Location (0xa0000005)\n");
            k++;
            curoffset = curoffset + 16;
        }
        else if(blksig == 0xa0000006) // darwin properties (788 bytes)
        {
            filecontents->append("\tSignature\t| Darwin Properties (0xa0000006)\n");
            uint8_t* dp = new uint8_t[260];
            dp = substr(tmpbuf, curoffset + 8, 260);
            filecontents->append("\tApp ID\t| " + std::string((char*)dp) + "\n");
            delete[] dp;
            k++;
            curoffset = curoffset + 788;
        }
        else if(blksig == 0xa0000007) // icon location (788 bytes)
        {
            filecontents->append("\tSignature\t| Icon Location (0xa0000007)\n");
            uint8_t* dp = new uint8_t[260];
            dp = substr(tmpbuf, curoffset + 8, 260);
            filecontents->append("\tIcon Location\t| " + std::string((char*)dp) + "\n");
            delete[] dp;
            k++;
            curoffset = curoffset + 788;
        }
        else if(blksig == 0xa0000008) // shim layer properties (variable)
        {
            filecontents->append("\tSignature\t| Shim Layer Properties (0xa0000008)\n");
            k++;
            curoffset = curoffset + blksize;
        }
        else if(blksig == 0xa0000009) // metadata property store (variable)
        {
            filecontents->append("\tSignature\t| Metadata Property Store (0xa0000009)\n");
            k++;
            curoffset = curoffset + blksize;
        }
        else if(blksig == 0xa000000b) // known folder location (28 bytes)
        {
            filecontents->append("\tSignature\t| Known Folder Location (0xa000000b)\n");
            k++;
            curoffset = curoffset + 28;
        }
        else if(blksig == 0xa000000c) // shell item identifiers list (variable)
        {
            filecontents->append("\tSignature\t| Shell Item Identifiers List (0xa000000c)\n");
            k++;
            curoffset = curoffset + blksize;
        }
    }
}
