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

QRawObjectList RawReader::read_creatures(QFileInfo path, int start) {
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

        TRACE << "Read " << lines.size() << " lines.";
        int NCreatures = start;
        while(!lines.isEmpty()) {
            RawObjectPtr obj = read_creature(lines, NCreatures);

            if(!obj.isNull())
            {
                result.append(obj);
                NCreatures++;
            }
        }
    }
    return result;
}

RawObjectPtr RawReader::read_creature(QStringList & lines, int N) {
    RawObjectPtr result;

    QString line = lines.first();
    lines.removeFirst();

    if(line.isEmpty())
        return result;

    if(line.trimmed().startsWith("[CREATURE:")) {
        result = RawObjectPtr(new RawObject);
        populate_creature_node_values(result, line, QString("%1").arg(N));
        populate_creature_sub_nodes(result, lines);
        TRACE << "Read object [" << result->get_name() << ":" << result->get_id() << "]";
    }
    return result;
}

void RawReader::populate_creature_node_values(const RawNodePtr & node, QString & line, QString id) {
    QString str = line.trimmed();
    //str = str.mid(1, str.length()-2);
    str = str.mid(1, str.indexOf("]")-1);
    QStringList strings = str.split(":", QString::SkipEmptyParts);
    node->name = strings[0];
    strings.insert(1, id);
    for(int i = 1; i < strings.size(); ++i) {
        node->values.append(strings[i]);
    }
}

void RawReader::populate_creature_sub_nodes(const RawNodePtr & node,  QStringList & lines) {
    bool done = false;
    while(!done && !lines.isEmpty()) {
        QString line = lines.front();
        lines.removeFirst();

        if(line.startsWith("[CREATURE:")) {
            done = true;
            lines.push_front(line);
        } else {
            if(line.trimmed().startsWith('[')) {
                    RawNodePtr subNode(new RawNode);
                    populate_node_values(subNode, line);
                    //TODO: popolare almeno le caste con i subNode relativi (una casta finisce dove ne inizia un'altra o dove ne viene selezionata una o dove vengono selezionate tutte
                    if ((subNode->name=="CASTE")||((subNode->name=="SELECT_CASTE")&&(subNode->values[0]!="ALL")))
                        populate_caste_sub_nodes(subNode, lines);
                    node->children.append(subNode);
                }
        }
    }
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
    //str = str.mid(1, str.length()-2);
    str = str.mid(1, str.indexOf("]")-1); //might be a good idea to do this elsewhere if we want to use more raws
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



void RawReader::populate_caste_sub_nodes(const RawNodePtr & node,  QStringList & lines) {
    bool done = false;
    while(!done && !lines.isEmpty()) {
        QString line = lines.front();
        lines.removeFirst();
        if((line.trimmed().startsWith("[CREATURE:"))||(line.trimmed().contains("[CASTE:"))||(line.trimmed().contains("[SELECT_CASTE:"))) {
            done = true;
            //lines.push_front(line);
            lines.push_front(line.remove(0,line.indexOf("[")));
        } else {
            //if(line.trimmed().startsWith('[')) {
            if(line.trimmed().contains('[')) {
                    RawNodePtr subNode(new RawNode);
                    //populate_node_values(subNode, line);
                    line = line.remove(0,line.indexOf("[")); //trim the junk before the [
                    populate_node_values(subNode, line);
                    node->children.append(subNode);
                }
        }
    }
}
