#include <QFile>
#include <QTextStream>
#include <QPair>
#include <QStringList>
#include "raws/rawreader.h"
#include "truncatingfilelogger.h"

QStringList read_lines(QTextStream & stream) {
    QStringList result;

    QString line = stream.readLine();
    while(!line.isNull()) {
        result.push_back(line);
        line = stream.readLine();
    }

    return result;
}

QRawObjectList RawReader::read_objects(QFileInfo path) {
    QRawObjectList result;

    if(!path.exists()) {
        LOGW << "File: " << path.fileName() << " does not exist.  Some functionality may not be available!";
        return result;
    }

    QFile file(path.absoluteFilePath());
    LOGD << "Reading from file: " << file.fileName();
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream stream( &file );
        QStringList lines = read_lines(stream);
        file.close();

        LOGD << "Read " << lines.size() << " lines.";

        while(!lines.isEmpty()) {
            RawObjectPtr obj = read_object(lines);

            if(!obj.isNull())
                result.append(obj);
        }
    }

    return result;
}

RawObjectPtr RawReader::read_object(QStringList & lines) {
    RawObjectPtr result;

    QString line = lines.first();
    lines.removeFirst();

    if(line.trimmed().startsWith('[')) {
        result = RawObjectPtr(new RawObject);
        populate_node_values(result, line);
        populate_sub_nodes(result, lines);
        TRACE << "Read object [" << result->get_name() << ":" << result->get_id() << "]";
    }

    return result;
}

void RawReader::populate_node_values(const RawNodePtr & node, QString & line) {
    QString str = line.trimmed();
    str = str.mid(1, str.length()-2);
    QStringList strings = str.split(":", QString::SkipEmptyParts);
    node->name = strings[0];
    for(int i = 1; i < strings.size(); ++i) {
        node->values.append(strings[i]);
    }
}

void RawReader::populate_sub_nodes(const RawNodePtr & node,  QStringList & lines, QString indent) {
    bool done = false;
    while(!done && !lines.isEmpty()) {
        QString line = lines.front();
        lines.removeFirst();

        if(line.trimmed().startsWith('[')) {
            if(!line.startsWith(indent)) {
                done = true;
                lines.push_front(line);
            } else {
                RawNodePtr subNode(new RawNode);
                populate_node_values(subNode, line);
                populate_sub_nodes(subNode, lines, indent + "\t");
                node->children.append(subNode);
            }
        }
    }
}


