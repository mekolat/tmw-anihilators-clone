/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <guichan/actionlistener.hpp>
#include <guichan/graphics.hpp>
#include <guichan/key.hpp>

#include "color.h"
#include "table.h"

#include "../configuration.h"

#include "../utils/dtor.h"

float GuiTable::mAlpha = config.getValue("guialpha", 0.8);

class GuiTableActionListener : public gcn::ActionListener
{
public:
    GuiTableActionListener(GuiTable *_table, gcn::Widget *_widget, int _row, int _column);

    virtual ~GuiTableActionListener();

    virtual void action(const gcn::ActionEvent& actionEvent);

protected:
    GuiTable *mTable;
    int mRow;
    int mColumn;
    gcn::Widget *mWidget;
};


GuiTableActionListener::GuiTableActionListener(GuiTable *table, gcn::Widget *widget, int row, int column) :
    mTable(table),
    mRow(row),
    mColumn(column),
    mWidget(widget)
{
    if (widget)
    {
        widget->addActionListener(this);
        widget->_setParent(table);
    }
}

GuiTableActionListener::~GuiTableActionListener()
{
    if (mWidget)
    {
        mWidget->removeActionListener(this);
        mWidget->_setParent(NULL);
    }
}

void GuiTableActionListener::action(const gcn::ActionEvent& actionEvent)
{
    mTable->setSelected(mRow, mColumn);
    mTable->distributeActionEvent();
}


GuiTable::GuiTable(TableModel *initial_model, gcn::Color background,
                   bool opacity) :
    mLinewiseMode(false),
    mWrappingEnabled(false),
    mOpaque(opacity),
    mBackgroundColor(background),
    mModel(NULL),
    mSelectedRow(0),
    mSelectedColumn(0),
    mTopWidget(NULL)
{
    setModel(initial_model);
    setFocusable(true);

    addMouseListener(this);
    addKeyListener(this);
}

GuiTable::~GuiTable()
{
    delete mModel;
}

TableModel *GuiTable::getModel() const
{
    return mModel;
}

void GuiTable::setModel(TableModel *new_model)
{
    if (mModel)
    {
        uninstallActionListeners();
        mModel->removeListener(this);
    }

    mModel = new_model;
    installActionListeners();

    if (new_model)
    {
        new_model->installListener(this);
        recomputeDimensions();
    }
}

void GuiTable::recomputeDimensions()
{
    int rows_nr = mModel->getRows();
    int columns_nr = mModel->getColumns();
    int width = 0;
    int height = 0;

    if (mSelectedRow >= rows_nr)
        mSelectedRow = rows_nr - 1;

    if (mSelectedColumn >= columns_nr)
        mSelectedColumn = columns_nr - 1;

    for (int i = 0; i < columns_nr; i++)
        width += getColumnWidth(i);

    height = getRowHeight() * rows_nr;

    setWidth(width);
    setHeight(height);
}

void GuiTable::setSelected(int row, int column)
{
    mSelectedColumn = column;
    mSelectedRow = row;
}

int GuiTable::getSelectedRow()
{
    return mSelectedRow;
}

int GuiTable::getSelectedColumn()
{
    return mSelectedColumn;
}

void GuiTable::setLinewiseSelection(bool linewise)
{
    mLinewiseMode = linewise;
}

int GuiTable::getRowHeight()
{
    if (mModel)
        return mModel->getRowHeight() + 1; // border
    else
        return 0;
}

int GuiTable::getColumnWidth(int i)
{
    if (mModel)
        return mModel->getColumnWidth(i) + 1; // border
    else
        return 0;
}

void GuiTable::setSelectedRow(int selected)
{
    if (!mModel)
    {
        mSelectedRow = -1;
    }
    else
    {
        if (selected < 0 && !mWrappingEnabled)
        {
            mSelectedRow = -1;
        }
        else if (selected >= mModel->getRows() && mWrappingEnabled)
        {
            mSelectedRow = 0;
        }
        else if ((selected >= mModel->getRows() && !mWrappingEnabled) ||
                 (selected < 0 && mWrappingEnabled))
        {
            mSelectedRow = mModel->getRows() - 1;
        }
        else
        {
            mSelectedRow = selected;
        }
    }
}

void GuiTable::setSelectedColumn(int selected)
{
    if (!mModel)
    {
        mSelectedColumn = -1;
    }
    else
    {
        if ((selected >= mModel->getColumns() && mWrappingEnabled) ||
                 (selected < 0 && !mWrappingEnabled))
        {
            mSelectedColumn = 0;
        }
        else if ((selected >= mModel->getColumns() && !mWrappingEnabled) ||
                 (selected < 0 && mWrappingEnabled))
        {
            mSelectedColumn = mModel->getColumns() - 1;
        }
        else
        {
            mSelectedColumn = selected;
        }
    }
}

void GuiTable::uninstallActionListeners(void)
{
    delete_all(mActionListeners);
    mActionListeners.clear();
}

void GuiTable::installActionListeners()
{
    if (!mModel)
        return;

    int rows = mModel->getRows();
    int columns = mModel->getColumns();

    for (int row = 0; row < rows; ++row)
        for (int column = 0; column < columns; ++column)
        {
            gcn::Widget *widget = mModel->getElementAt(row, column);
            mActionListeners.push_back(new GuiTableActionListener(this, widget,
                                                                  row, column));
        }

    _setFocusHandler(_getFocusHandler()); // propagate focus handler to widgets
}

// -- widget ops
void GuiTable::draw(gcn::Graphics* graphics)
{
    if (!mModel)
        return;

    if (config.getValue("guialpha", 0.8) != mAlpha)
        mAlpha = config.getValue("guialpha", 0.8);

    if (mOpaque)
    {
        const int red = getBackgroundColor().r;
        const int green = getBackgroundColor().g;
        const int blue = getBackgroundColor().b;
        const int alpha = (int)(mAlpha * 255.0f);
        graphics->setColor(gcn::Color(red, green, blue, alpha));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    // First, determine how many rows we need to draw, and where we should start.
    int first_row = -(getY() / getRowHeight());

    if (first_row < 0)
        first_row = 0;

    int rows_nr = 1 + (getHeight() / getRowHeight()); // May overestimate by one.

    int max_rows_nr = mModel->getRows() - first_row; // clip if neccessary:
    if (max_rows_nr < rows_nr)
        rows_nr = max_rows_nr;

    // Now determine the first and last column
    // Take the easy way out; these are usually bounded and all visible.
    int first_column = 0;
    int last_column = mModel->getColumns() - 1;

    // Set up everything for drawing
    int height = getRowHeight();
    int y_offset = first_row * height;

    for (int r = first_row; r < first_row + rows_nr; ++r)
    {
        int x_offset = 0;

        for (int c = first_column; c <= last_column; ++c)
        {
            gcn::Widget *widget = mModel->getElementAt(r, c);
            int width = getColumnWidth(c);
            if (widget)
            {
                gcn::Rectangle bounds(x_offset, y_offset, width, height);

                if (widget == mTopWidget)
                {
                    bounds.height = widget->getHeight();
                    bounds.width = widget->getWidth();
                }

                widget->setDimension(bounds);

                if (!mLinewiseMode && c == mSelectedColumn && r == mSelectedRow)
                {
                    bool valid;
                    const int red =
                        (textColor->getColor('H', valid) >> 16) & 0xFF;
                    const int green =
                        (textColor->getColor('H', valid) >> 8) & 0xFF;
                    const int blue = textColor->getColor('H', valid) & 0xFF;
                    const int alpha = (int)(mAlpha * 127.0f);

                    graphics->setColor(gcn::Color(red, green, blue, alpha));
                    graphics->fillRectangle(bounds);
                }

                graphics->pushClipArea(bounds);
                widget->draw(graphics);
                graphics->popClipArea();
            }

            x_offset += width;
        }

        if (mLinewiseMode && r == mSelectedRow)
        {
            bool valid;
            const int red =
                (textColor->getColor('H', valid) >> 16) & 0xFF;
            const int green =
                (textColor->getColor('H', valid) >> 8) & 0xFF;
            const int blue = textColor->getColor('H', valid) & 0xFF;
            const int alpha = (int)(mAlpha * 127.0f);

            graphics->setColor(gcn::Color(red, green, blue, alpha));
            graphics->fillRectangle(gcn::Rectangle(0, y_offset,
                                                   x_offset, height));
        }

        y_offset += height;
    }

    if (mTopWidget)
    {
        gcn::Rectangle bounds = mTopWidget->getDimension();
        graphics->pushClipArea(bounds);
        mTopWidget->draw(graphics);
        graphics->popClipArea();
    }
}

void GuiTable::moveToTop(gcn::Widget *widget)
{
    gcn::Widget::moveToTop(widget);
    mTopWidget = widget;
}

void GuiTable::moveToBottom(gcn::Widget *widget)
{
    gcn::Widget::moveToBottom(widget);
    if (widget == mTopWidget)
        mTopWidget = NULL;
}

gcn::Rectangle GuiTable::getChildrenArea()
{
    return gcn::Rectangle(0, 0, getWidth(), getHeight());
}

// -- KeyListener notifications
void GuiTable::keyPressed(gcn::KeyEvent& keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == gcn::Key::ENTER || key.getValue() == gcn::Key::SPACE)
    {
        distributeActionEvent();
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::UP)
    {
        setSelectedRow(mSelectedRow - 1);
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::DOWN)
    {
        setSelectedRow(mSelectedRow + 1);
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::LEFT)
    {
        setSelectedColumn(mSelectedColumn - 1);
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::RIGHT)
    {
        setSelectedColumn(mSelectedColumn + 1);
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::HOME)
    {
        setSelectedRow(0);
        setSelectedColumn(0);
        keyEvent.consume();
    }
    else if (key.getValue() == gcn::Key::END)
    {
        setSelectedRow(mModel->getRows() - 1);
        setSelectedColumn(mModel->getColumns() - 1);
        keyEvent.consume();
    }
}

// -- MouseListener notifications
void GuiTable::mousePressed(gcn::MouseEvent& mouseEvent)
{
    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        int row = getRowForY(mouseEvent.getY());
        int column = getColumnForX(mouseEvent.getX());

        if (row > -1 && column > -1 &&
            row < mModel->getRows() && column < mModel->getColumns())
        {
            mSelectedColumn = column;
            mSelectedRow = row;
        }

        distributeActionEvent();
    }
}

void GuiTable::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    if (isFocused())
    {
        if (getSelectedRow() > 0 || (getSelectedRow() == 0 && mWrappingEnabled))
        {
            setSelectedRow(getSelectedRow() - 1);
        }

        mouseEvent.consume();
    }
}

void GuiTable::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    if (isFocused())
    {
        setSelectedRow(getSelectedRow() + 1);

        mouseEvent.consume();
    }
}

void GuiTable::mouseDragged(gcn::MouseEvent& mouseEvent)
{
}

// -- TableModelListener notifications
void GuiTable::modelUpdated(bool completed)
{
    if (completed)
    {
        recomputeDimensions();
        installActionListeners();
    }
    else
    { // before the update?
        mTopWidget = NULL; // No longer valid in general
        uninstallActionListeners();
    }
}

gcn::Widget *GuiTable::getWidgetAt(int x, int y)
{
    int row = getRowForY(y);
    int column = getColumnForX(x);

    if (mTopWidget && mTopWidget->getDimension().isPointInRect(x, y))
        return mTopWidget;

    if (row > -1 && column > -1)
    {
        gcn::Widget *w = mModel->getElementAt(row, column);
        if (w && w->isFocusable())
            return w;
        else
            return NULL; // Grab the event locally
    }
    else
        return NULL;
}

int GuiTable::getRowForY(int y)
{
   int row = y / getRowHeight();

   if (row < 0 || row >= mModel->getRows())
       return -1;
   else
       return row;
}

int GuiTable::getColumnForX(int x)
{
    int column;
    int delta = 0;

    for (column = 0; column < mModel->getColumns(); column++)
    {
        delta += getColumnWidth(column);
        if (x <= delta)
            break;
    }

    if (column < 0 || column >= mModel->getColumns())
        return -1;
    else
        return column;
}

void GuiTable::_setFocusHandler(gcn::FocusHandler* focusHandler)
{
    gcn::Widget::_setFocusHandler(focusHandler);

    if (mModel) {
        for (int r = 0; r < mModel->getRows(); ++r) {
            for (int c = 0; c < mModel->getColumns(); ++c) {
                gcn::Widget *w = mModel->getElementAt(r, c);
                if (w)
                    w->_setFocusHandler(focusHandler);
            }
        }
    }
}
