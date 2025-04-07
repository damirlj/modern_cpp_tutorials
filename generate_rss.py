import os
import subprocess
import xml.etree.ElementTree as ET
from xml.dom import minidom
from datetime import datetime

# Constants
docs_folder = "docs"
rss_file = os.path.join(docs_folder, "rss.xml")

def get_git_last_modified_time(file_path):
    try:
        output = subprocess.check_output(
            ["git", "log", "-1", "--format=%ct", "--", file_path],
            text=True
        ).strip()
        return int(output)
    except Exception:
        return 0

def get_effective_mod_time(file_path):
    git_time = get_git_last_modified_time(file_path)
    return git_time if git_time != 0 else int(os.path.getmtime(file_path))

def format_rss_date(unix_timestamp):
    return datetime.utcfromtimestamp(unix_timestamp).strftime("%a, %d %b %Y %H:%M:%S GMT")

def create_empty_rss_file(file_path):
    root = ET.Element("rss", version="2.0")
    channel = ET.SubElement(root, "channel")
    ET.SubElement(channel, "title").text = "Modern C++ Tutorials"
    ET.SubElement(channel, "link").text = "https://github.com/damirlj/modern_cpp_tutorials"
    ET.SubElement(channel, "description").text = "RSS feed for Modern C++ Tutorials"
    tree = ET.ElementTree(root)
    tree.write(file_path, encoding="utf-8", xml_declaration=True)
    print(f"Created new RSS file: {file_path}")

def is_file_updated(mod_time, existing_pub_date):
    try:
        existing_time = datetime.strptime(existing_pub_date, "%a, %d %b %Y %H:%M:%S GMT").timestamp()
        return mod_time > existing_time
    except Exception as e:
        print(f"[Error] Failed to parse pubDate '{existing_pub_date}': {e}")
        return True

def write_rss_feed(file_path, tree, channel):
    # Sort items by pubDate timestamp descending
    def item_timestamp(item):
        pub_date = item.find("pubDate").text
        try:
            return datetime.strptime(pub_date, "%a, %d %b %Y %H:%M:%S GMT").timestamp()
        except:
            return 0

    sorted_items = sorted(channel.findall("item"), key=item_timestamp, reverse=True)
    channel.clear()
    for item in sorted_items:
        channel.append(item)

    xml_str = ET.tostring(tree.getroot(), encoding="utf-8", method="xml").decode("utf-8")
    pretty_xml = minidom.parseString(xml_str).toprettyxml(indent="  ")
    xml_lines = [line for line in pretty_xml.splitlines() if line.strip()]
    with open(file_path, 'w', encoding="utf-8") as f:
        f.write('\n'.join(xml_lines))

    print("RSS feed updated.")

# Ensure RSS file exists
if not os.path.exists(rss_file):
    create_empty_rss_file(rss_file)

# Load existing RSS tree
try:
    tree = ET.parse(rss_file)
    root = tree.getroot()
    channel = root.find("channel")
    existing_items = {
        item.find("link").text: item
        for item in channel.findall("item")
    }
except ET.ParseError:
    print("Failed to parse existing RSS file.")
    exit(1)

# Collect .pdf files
pdf_files = [
    os.path.join(root, file)
    for root, _, files in os.walk(docs_folder)
    for file in files if file.endswith(".pdf")
]

changes_made = False

for pdf in pdf_files:
    relative_path = os.path.relpath(pdf, start=docs_folder)
    relative_path = relative_path.replace(os.sep, "/")
    link = f"https://github.com/damirlj/modern_cpp_tutorials/blob/main/docs/{relative_path}"
    title = os.path.splitext(os.path.basename(pdf))[0].replace("_", " ")
    mod_time = get_effective_mod_time(pdf)
    pub_date = format_rss_date(mod_time)

    if link in existing_items:
        item = existing_items[link]
        if is_file_updated(mod_time, item.find("pubDate").text):
            item.find("pubDate").text = pub_date
            print(f"Updated: {title}")
            changes_made = True
    else:
        item = ET.SubElement(channel, "item")
        ET.SubElement(item, "title").text = title
        ET.SubElement(item, "link").text = link
        ET.SubElement(item, "pubDate").text = pub_date
        print(f"Added: {title}")
        changes_made = True

if changes_made:
    write_rss_feed(rss_file, tree, channel)
else:
    print("No changes detected.")
