import os
import xml.etree.ElementTree as ET
import uuid

# This script scans your disk and builds a .filters file
# that exactly matches your folder structure for ALL file types.

# --- Configuration ---
# Define all the file types the script should recognize.
HEADER_EXTENSIONS = {".h", ".hpp", ".hxx", ".inl"}
SOURCE_EXTENSIONS = {".c", ".cpp", ".cxx"}
IMAGE_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tga"}

# Files and extensions to completely ignore during the scan.
EXCLUDED_FILES = {"vcxprojmaker.py"} # Add the script's own name here
EXCLUDED_EXTENSIONS = {".sln", ".vcxproj", ".filters", ".user", ".pyc"}

# --- Script ---
filters_file = "OpenGlt.vcxproj.filters"
scan_directory = "."
print(f"🔍 Scanning '{os.path.abspath(scan_directory)}' for ALL files to map your disk structure...")

try:
    discovered_files = []
    all_folders = set()

    for dirpath, _, filenames in os.walk(scan_directory):
        if os.path.basename(dirpath).startswith('.'):
            continue
            
        for filename in filenames:
            # Skip any file or extension in the exclusion lists.
            if filename in EXCLUDED_FILES or os.path.splitext(filename)[1].lower() in EXCLUDED_EXTENSIONS:
                continue

            extension = os.path.splitext(filename)[1].lower()
            file_type = None

            # Assign a Visual Studio item type based on the file extension.
            if extension in HEADER_EXTENSIONS:
                file_type = "ClInclude"
            elif extension in SOURCE_EXTENSIONS:
                file_type = "ClCompile"
            elif extension in IMAGE_EXTENSIONS:
                file_type = "Image"
            else:
                # This is the crucial part: everything else is treated as generic content.
                # The <None> tag includes the file in the project without compiling it.
                # Perfect for DLLs, shaders (.glsl), text files, etc.
                file_type = "None"
            
            relative_path = os.path.relpath(os.path.join(dirpath, filename), scan_directory)
            cleaned_path = relative_path.replace("\\", "/")
            discovered_files.append((cleaned_path, file_type))
            
            folder_path = os.path.dirname(cleaned_path)
            if folder_path:
                all_folders.add(folder_path)

    if not discovered_files:
        print("⚠️ No relevant files were found. Aborting.")
        exit()

    print(f"✅ Found {len(discovered_files)} files in {len(all_folders)} folders.")
    
    # Build the .filters file from the comprehensive scanned data.
    ns = "http://schemas.microsoft.com/developer/msbuild/2003"
    ET.register_namespace('', ns)
    filters_root = ET.Element("Project", {"ToolsVersion": "4.0", "xmlns": ns})

    # Create <Filter> elements for folders.
    filter_group = ET.SubElement(filters_root, "ItemGroup")
    for folder_path in sorted(list(all_folders)):
        filter_elem = ET.SubElement(filter_group, "Filter", {"Include": folder_path})
        unique_id = "{" + str(uuid.uuid4()).upper() + "}"
        ET.SubElement(filter_elem, "UniqueIdentifier").text = unique_id

    # Create item groups to map each file to its folder.
    file_group = ET.SubElement(filters_root, "ItemGroup")
    for file_path, file_type in discovered_files:
        file_entry = ET.SubElement(file_group, file_type, {"Include": file_path})
        folder_path = os.path.dirname(file_path)
        if folder_path:
            ET.SubElement(file_entry, "Filter").text = folder_path

    # Write the result to the .filters file.
    ET.ElementTree(filters_root).write(filters_file, encoding="utf-8", xml_declaration=True)
    print(f"🎉 Successfully generated '{filters_file}'! All files should now be correctly organized.")

except Exception as e:
    print(f"❌ An unexpected error occurred: {e}")