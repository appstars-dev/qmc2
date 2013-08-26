#!/bin/sh
# Create man-pages using txt2man.sh

MAN_SOURCE_FOLDER=$1
if [ "$MAN_SOURCE_FOLDER" == "" ]; then
	echo "Usage: $0 <man_source_folder> <version> [<config_path> [<sys_conf_dir>]]"
	exit 1
fi

VERSION=$2
if [ "$VERSION" == "" ]; then
	echo "Usage: $0 <man_source_folder> <version> [<config_path> [<sys_conf_dir>]]"
	exit 1
fi

TXT2MAN=$(cd $(dirname "$0"); pwd)/txt2man.sh

CONFIG_PATH=~/.qmc2
if [ "$3" != "" ]; then
	CONFIG_PATH=$3
fi

SYS_CONF_DIR=/etc
if [ "$4" != "" ]; then
	SYS_CONF_DIR=$4
fi

cd $MAN_SOURCE_FOLDER > /dev/null

for ctl_file in $(ls *.ctl); do
	. ./$ctl_file
	ctl_file_basename=$(basename $ctl_file .ctl)
	man_source=$ctl_file_basename.man.text
	man_target=$ctl_file_basename.$man_section
	man_compressed_target=$man_target.gz
	echo "Converting '$man_source' to '$man_target'"
	cat $man_source | sed -e "s|SYS_CONF_DIR|$SYS_CONF_DIR|g" -e "s|CONFIG_PATH|$CONFIG_PATH|g" | $TXT2MAN -t "$title_name" -v "$volume_name" -r "$VERSION" > $man_target
	echo "Compressing '$man_target' to '$man_compressed_target'"
	gzip -f $man_target
	for man_page_link in $man_pages; do
		if [ "$man_page_link" != "$ctl_file_basename" ]; then
			link_name=$man_page_link.$man_section.gz
			echo "Sym-linking '$man_compressed_target' to '$link_name'"
			ln -s -f $man_compressed_target $link_name
		fi
	done
done

cd - > /dev/null
