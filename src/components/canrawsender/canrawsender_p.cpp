#include "canrawsender_p.h"
#include "canrawsender.h"
#include "log.h"
#include <QJsonArray>

void CanRawSenderPrivate::setSimulationState(bool state)
{
    _simulationState = state;
    for (auto& iter : _lines) {
        iter->SetSimulationState(state);
    }
}

void CanRawSenderPrivate::saveSettings(QJsonObject& json) const
{
    /*
        QJsonObject jSortingObject;
        QJsonArray lineArray;
        writeColumnsOrder(json);
        writeSortingRules(jSortingObject);
        json["sorting"] = std::move(jSortingObject);

    for (const auto& lineItem : _lines) {
        QJsonObject lineObject;
        lineItem->Line2Json(lineObject);
        lineArray.append(std::move(lineObject));
    }

    json["content"] = std::move(lineArray);
    */
}

int CanRawSenderPrivate::getLineCount() const
{
    // return _lines.size();
    return _tvModel.rowCount();
}

void CanRawSenderPrivate::writeColumnsOrder(QJsonObject& json) const
{
    QJsonArray columnList;
    for (const auto& column : _columnsOrder) {
        columnList.append(column);
    }
    json["columns"] = std::move(columnList);
}

void CanRawSenderPrivate::writeSortingRules(QJsonObject& json) const
{
    json["currentIndex"] = _currentIndex;
}

void CanRawSenderPrivate::removeRowsSelectedByMouse()
{
    QModelIndexList IndexList = _ui.getSelectedRows();
    std::list<QModelIndex> tmp = IndexList.toStdList();

    tmp.sort(); // List must to be sorted and reversed because erasing started from last row
    tmp.reverse();

    for (QModelIndex n : tmp) {
        _tvModel.removeRow(n.row()); // Delete line from table view
        //_lines.erase(_lines.begin() + n.row()); // Delete lines also from collection
        // TODO: check if works when the collums was sorted before
    }
}

void CanRawSenderPrivate::addNewItem()
{
    QList<QStandardItem*> list{};
    list.append(new QStandardItem(QString::number(_rowID++)));
    list.append(new QStandardItem(QString::number(0)));
    list.append(new QStandardItem(QString::number(0)));
    list.append(new QStandardItem(QString::number(0)));

    list.append(new QStandardItem(QString::number(0))); // checkbox
    list.append(new QStandardItem(QString::number(0))); // send button

    _tvModel.appendRow(list);
    //auto newLine = std::make_unique<NewLineManager>(q_ptr, _simulationState, _nlmFactory);

    int lastRowIndex = _tvModel.rowCount() - 1;
    QModelIndex index1 = _sortModel.index(lastRowIndex, 4); // checkbox
    QModelIndex index2 = _sortModel.index(lastRowIndex, 5); // send button
    _ui.setWidgetPersistent(index1); // set widget always visible
    _ui.setWidgetPersistent(index2);

    /*
    auto newLine = std::make_unique<NewLineManager>(q_ptr, _simulationState, _nlmFactory);
    using It = NewLineManager::ColNameIterator;
    for (NewLineManager::ColName ii : It{ NewLineManager::ColName::IdLine }) {
        _ui.setIndexWidget(
            _tvModel.index(_tvModel.rowCount() - 1, static_cast<int>(ii)), newLine->GetColsWidget(It{ ii }));

    _lines.push_back(std::move(newLine));
        }
    */
}
/**
*   @brief  Function sets current sort settings and calls actual sort function
*   @param  clickedIndex index of last clicked column
*/
void CanRawSenderPrivate::sort(const int clickedIndex)
{
    _currentSortOrder = _ui.getSortOrder();
    _sortIndex = clickedIndex;

    if (_prevIndex == clickedIndex) {
        if (_currentSortOrder == Qt::DescendingOrder) {
            _ui.setSorting(_sortIndex, Qt::DescendingOrder);
        } else {
            _ui.setSorting(0, Qt::AscendingOrder);
            _prevIndex = 0;
            _sortIndex = 0;
        }
    } else {
        _ui.setSorting(_sortIndex, Qt::AscendingOrder);
        _prevIndex = clickedIndex;
    }
}

bool CanRawSenderPrivate::columnAdopt(QJsonObject const& json)
{
    auto columnIter = json.find("columns");
    if (columnIter == json.end()) {
        cds_error("Columns item not found it");
        return false;
    }

    if (columnIter.value().type() != QJsonValue::Array) {
        cds_error("Columns format is different then array");
        return false;
    }

    auto colArray = json["columns"].toArray();
    if (colArray.size() != 5) {
        cds_error("Columns array size is {} - must by 5!", std::to_string(colArray.size()));
        return false;
    }
    if (colArray.contains("Id") == false) {
        cds_error("Columns array does not contain Id field.");
        return false;
    }
    if (colArray.contains("Data") == false) {
        cds_error("Columns array does not contain Data field.");
        return false;
    }
    if (colArray.contains("Loop") == false) {
        cds_error("Columns array does not contain Loop field.");
        return false;
    }
    if (colArray.contains("Interval") == false) {
        cds_error("Columns array does not contain Interval field.");
        return false;
    }

    cds_info("Columns validation is finished successfully.");
    return true;
}

bool CanRawSenderPrivate::sortingAdopt(QJsonObject const& json)
{
    auto sortingIter = json.find("sorting");
    if (sortingIter == json.end()) {
        cds_error("Sorting item not found it");
        return false;
    }
    if (sortingIter.value().type() != QJsonValue::Object) {
        cds_error("Sorting format is different then object");
        return false;
    }

    auto sortingObj = json["sorting"].toObject();
    if (sortingObj.count() != 1) {
        cds_error("Sorting object count {} is different then 1.", std::to_string(sortingObj.count()));
        return false;
    }
    if (sortingObj.contains("currentIndex") == false) {
        cds_error("Sorting object does not contain currentIndex.");
        return false;
    }
    if (sortingObj["currentIndex"].isDouble() == false) {
        cds_error("currentIndex format in sorting object is incorrect.");
        return false;
    }

    cds_info("Sorting validation is finished successfully.");

    auto newIdx = sortingObj["currentIndex"].toInt();
    if ((newIdx < 0) || (newIdx > 5)) {
        cds_error("currentIndex data '{}' is out of range.", std::to_string(newIdx));
        return false;
    }

    _currentIndex = newIdx;
    cds_debug("currentIndex data is adopted new value = {}.", std::to_string(_currentIndex));

    return true;
}

bool CanRawSenderPrivate::contentAdopt(QJsonObject const& json)
{
    QString data, id, interval;
    bool loop;

    auto contentIter = json.find("content");
    if (contentIter == json.end()) {
        cds_error("Content item not found it");
        return false;
    }
    if (contentIter.value().type() != QJsonValue::Array) {
        cds_error("Content format is different then array");
        return false;
    }

    auto contentArray = json["content"].toArray();
    for (auto ii = 0; ii < contentArray.count(); ++ii) {
        auto line = contentArray[ii].toObject();

        data.clear();
        id.clear();
        interval.clear();
        loop = false;

        // Data
        if (line.contains("data")) {
            if (line["data"].isString() == true) {
                data = line["data"].toString();
            } else {
                cds_error("Data does not contain a string format.");
                return false;
            }
        } else {
            cds_info("Data is not available.");
        }
        // Id
        if (line.contains("id")) {
            if (line["id"].isString() == true) {
                id = line["id"].toString();
            } else {
                cds_error("Id does not contain a string format.");
                return false;
            }
        } else {
            cds_info("Id is not available.");
        }
        // Interval
        if (line.contains("interval")) {
            if (line["interval"].isString() == true) {
                interval = line["interval"].toString();
            } else {
                cds_error("Interval does not contain a string format.");
                return false;
            }
        } else {
            cds_info("Interval is not available.");
        }
        // Loop
        if (line.contains("loop")) {
            if (line["loop"].isBool() == true) {
                loop = line["loop"].toBool();
            } else {
                cds_error("Loop does not contain a bool format.");
                return false;
            }
        } else {
            cds_info("Loop is not available.");
        }

        // Add new lines with dependencies
        addNewItem();
        /*
        if (_lines.back()->RestoreLine(id, data, interval, loop) == false) {
            _tvModel.removeRow(_lines.size() - 1); // Delete line from table view
            _lines.erase(_lines.end() - 1); // Delete lines also from collection
            cds_warn("Problem with a validation of line occurred.");
        } else {
            cds_info("New line was adopted correctly.");
        }*/
    }

    return true;
}

bool CanRawSenderPrivate::restoreConfiguration(const QJsonObject& json)
{
    if (columnAdopt(json) == false) {
        return false;
    }
    if (contentAdopt(json) == false) {
        return false;
    }
    if (sortingAdopt(json) == false) {
        return false;
    }

    return true;
}
