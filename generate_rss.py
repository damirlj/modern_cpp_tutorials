import os
import subprocess
import xml.etree.ElementTree as ET
from xml.dom import minidom
from datetime import datetime


# Define the RSS file and folder containing articles
docs_folder = "docs"
rss_file = os.path.join(docs_folder, "rss.xml")  # Adjust path to rss.xml

# Use local time instead of GMT
# current_date = datetime.now().strftime("%a, %d %b %Y %H:%M:%S %z")  # Local time with timezone offset
# Get the current timestamp in GMT
current_date = datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")

def get_git_last_modified_time(file_path):
    try:
        # Get the last commit timestamp in UTC for the file
        output = subprocess.check_output(
            ["git", "log", "-1", "--format=%ct", "--", file_path],
            text=True
        ).strip()
        return int(output)  # UNIX timestamp
    except Exception as e:
        print(f"Failed to get git modified time for {file_path}: {e}")
        return 0

def create_empty_rss_file(file_path):
    """
    Create an empty RSS file with the basic structure if it doesn't exist.
    :param file_path: Path to the RSS file.
    """
    root = ET.Element("rss", version="2.0")
    channel = ET.SubElement(root, "channel")
    ET.SubElement(channel, "title").text = "Modern C++ Tutorials"
    ET.SubElement(channel, "link").text = "https://github.com/damirlj/modern_cpp_tutorials"
    ET.SubElement(channel, "description").text = "RSS feed for Modern C++ Tutorials"
    tree = ET.ElementTree(root)
    tree.write(file_path, encoding="utf-8", xml_declaration=True)
    print(f"Created new RSS file: {file_path}")

def is_file_updated(file_path, existing_pub_date):
    """
    Check if the file has been updated based on its modification timestamp.
    :param file_path: Path to the file.
    :param existing_pub_date: Existing publication date in RSS feed.
    :return: True if the file has been updated, False otherwise.
    """
    # file_mod_time = os.path.getmtime(file_path)
    git_mod_time = get_git_last_modified_time(file_path)
    existing_time = datetime.strptime(existing_pub_date, "%a, %d %b %Y %H:%M:%S GMT").timestamp()
    updated = git_mod_time > existing_time
    if updated:
        print(f"File {file_path} has been updated (mod_time: {git_mod_time}, existing_time: {existing_time})")
    return updated


def write_rss_feed(file_path, tree, channel):
    """
    Write the updated RSS feed to the file with proper formatting using minidom.
    :param file_path: Path to the RSS file.
    :param tree: ElementTree object of the XML.
    :param channel: Channel element containing the items.
    """
    # Sort the items by pubDate in descending order (latest first)
    channel_items = sorted(channel.findall("item"), key=lambda x: x.find("pubDate").text, reverse=True)

    # Rebuild the channel with sorted items
    channel.clear()
    for item in channel_items:
        channel.append(item)

    # Save the updated RSS feed to a string and format using minidom
    xml_str = ET.tostring(tree.getroot(), encoding="utf-8", method="xml").decode("utf-8")
    
    # Parse the XML string with minidom for proper indentation
    xml_str = minidom.parseString(xml_str).toprettyxml(indent="  ")

    # We need to make sure the first line only contains the XML declaration, without extra blank lines
    xml_lines = xml_str.splitlines()
    xml_lines = [line for line in xml_lines if line.strip()]  # Remove empty lines
    xml_str = '\n'.join(xml_lines)

    # Write the formatted XML back to the file
    with open(file_path, 'w', encoding="utf-8") as file:
        file.write(xml_str)

    print("RSS feed updated.")

# Check if the RSS file exists, and create it if necessary
if not os.path.exists(rss_file):
    create_empty_rss_file(rss_file)

# Load the existing RSS feed
try:
    tree = ET.parse(rss_file)
    root = tree.getroot()
    channel = root.find("channel")
    existing_items = {
        item.find("link").text: item 
        for item in channel.findall("item")
    }
    if not existing_items:
        print("The existing_items dictionary is empty.")
except ET.ParseError:
    print("Error parsing the existing RSS file. The file might be corrupted.")
    exit(1)

# Get the list of PDF files in the docs folder (including subfolders)
pdf_files = [
    os.path.join(root, f)
    for root, dirs, files in os.walk(docs_folder)
    for f in files if f.lower().endswith(".pdf")
]

# Track changes
changes_made = False

# Process each PDF file
for pdf in sorted(pdf_files):
    relative_path = os.path.relpath(pdf, start=docs_folder)  # This will include docs/ and subfolders
    relative_path = relative_path.replace(os.sep, "/")  # Normalize to URL format
    link = f"https://github.com/damirlj/modern_cpp_tutorials/blob/main/docs/{relative_path}"

    # Check if the item already exists
    if link in existing_items:
        item = existing_items[link]
        pub_date = item.find("pubDate").text

        # Check if the file is updated based on modification time
        if is_file_updated(pdf, pub_date):
            pub_date = current_date  # Update pubDate for modified items
            item.find("pubDate").text = pub_date  # Update the pubDate of the existing item
            changes_made = True
    else:
        # Assign the current date for new items
        pub_date = current_date
        item = ET.Element("item")
        ET.SubElement(item, "title").text = relative_path
        ET.SubElement(item, "link").text = link
        ET.SubElement(item, "guid").text = link
        ET.SubElement(item, "pubDate").text = pub_date
        channel.append(item)
        changes_made = True
        print(f"Adding new file: {pdf}")

# Save the updated RSS feed if changes were made
if changes_made:
    write_rss_feed(rss_file, tree, channel)
else:
    print("No changes made to the RSS feed.")
