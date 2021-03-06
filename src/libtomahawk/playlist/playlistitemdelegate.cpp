/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#include "playlistitemdelegate.h"

#include <QApplication>
#include <QPainter>

#include "query.h"
#include "result.h"
#include "artist.h"
#include "source.h"
#include "sourcelist.h"

#include "trackmodel.h"
#include "trackmodelitem.h"
#include "trackproxymodel.h"
#include "trackview.h"
#include "trackheader.h"

#include "utils/tomahawkutils.h"
#include "utils/logger.h"

#define PLAYING_ICON QString( RESPATH "images/now-playing-speaker.png" )
#define ARROW_ICON QString( RESPATH "images/info.png" )

using namespace Tomahawk;


PlaylistItemDelegate::PlaylistItemDelegate( TrackView* parent, TrackProxyModel* proxy )
    : QStyledItemDelegate( (QObject*)parent )
    , m_view( parent )
    , m_model( proxy )
{
    m_nowPlayingIcon = QPixmap( PLAYING_ICON );
    m_arrowIcon = QPixmap( ARROW_ICON );

    m_topOption = QTextOption( Qt::AlignTop );
    m_topOption.setWrapMode( QTextOption::NoWrap );

    m_bottomOption = QTextOption( Qt::AlignBottom );
    m_bottomOption.setWrapMode( QTextOption::NoWrap );

    m_centerOption = QTextOption( Qt::AlignVCenter );
    m_centerOption.setWrapMode( QTextOption::NoWrap );

    m_defaultAvatar = TomahawkUtils::createAvatarFrame( QPixmap( RESPATH "images/user-avatar.png" ) );
}


void
PlaylistItemDelegate::updateRowSize( const QModelIndex& index )
{
    emit sizeHintChanged( index );
}


QSize
PlaylistItemDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QSize size = QStyledItemDelegate::sizeHint( option, index );

    if ( index.isValid() )
    {
        int style = index.data( TrackModel::StyleRole ).toInt();
        if ( style == TrackModel::Short || style == TrackModel::ShortWithAvatars )
            size.setHeight( 44 );
    }

    return size;
}


QWidget*
PlaylistItemDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_UNUSED( parent );
    Q_UNUSED( option );
    Q_UNUSED( index );
    return 0;
}


void
PlaylistItemDelegate::prepareStyleOption( QStyleOptionViewItemV4* option, const QModelIndex& index, TrackModelItem* item ) const
{
    initStyleOption( option, index );

    if ( item->isPlaying() )
    {
        option->palette.setColor( QPalette::Highlight, option->palette.color( QPalette::Mid ) );
        option->state |= QStyle::State_Selected;
    }

    if ( option->state & QStyle::State_Selected )
    {
        option->palette.setColor( QPalette::Text, option->palette.color( QPalette::HighlightedText ) );
    }
    else
    {
        float opacity = 0.0;
        if ( item->query()->results().count() )
            opacity = item->query()->results().first()->score();

        opacity = qMax( (float)0.3, opacity );
        QColor textColor = TomahawkUtils::alphaBlend( option->palette.color( QPalette::Text ), option->palette.color( QPalette::BrightText ), opacity );

        option->palette.setColor( QPalette::Text, textColor );
    }
}


void
PlaylistItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    int style = index.data( TrackModel::StyleRole ).toInt();
    switch ( style )
    {
        case TrackModel::Detailed:
            paintDetailed( painter, option, index );
            break;

        case TrackModel::Short:
            paintShort( painter, option, index );
            break;
        case TrackModel::ShortWithAvatars:
            paintShort( painter, option, index, true );
    }
}


void
PlaylistItemDelegate::paintShort( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, bool useAvatars ) const
{
    TrackModelItem* item = m_model->itemFromIndex( m_model->mapToSource( index ) );
    Q_ASSERT( item );

    QStyleOptionViewItemV4 opt = option;
    prepareStyleOption( &opt, index, item );
    opt.text.clear();

    qApp->style()->drawControl( QStyle::CE_ItemViewItem, &opt, painter );

    if ( m_view->header()->visualIndex( index.column() ) > 0 )
        return;

    QPixmap pixmap;
    QString artist, track, upperText, lowerText;
    source_ptr source = item->query()->playedBy().first;

    if ( item->query()->results().count() )
    {
        artist = item->query()->results().first()->artist()->name();
        track = item->query()->results().first()->track();
    }
    else
    {
        artist = item->query()->artist();
        track = item->query()->track();
    }

    if ( source.isNull() )
    {
        upperText = artist;
        lowerText = track;
    }
    else
    {
        upperText = QString( "%1 - %2" ).arg( artist ).arg( track );
        QString playtime = TomahawkUtils::ageToString( QDateTime::fromTime_t( item->query()->playedBy().second ), true );

        if ( source == SourceList::instance()->getLocal() )
            lowerText = QString( "played %1 by you" ).arg( playtime );
        else
            lowerText = QString( "played %1 by %2" ).arg( playtime ).arg( source->friendlyName() );

        if ( useAvatars )
            pixmap = source->avatar( Source::FancyStyle );
    }

    if ( pixmap.isNull() && !useAvatars )
        pixmap = QPixmap( RESPATH "images/track-placeholder.png" );
    else if ( pixmap.isNull() && useAvatars )
        pixmap = m_defaultAvatar;

    painter->save();
    {
        QRect r = opt.rect.adjusted( 3, 6, 0, -6 );

        // Paint Now Playing Speaker Icon
        if ( item->isPlaying() )
        {
            r.adjust( 0, 0, 0, 0 );
            QRect npr = r.adjusted( 3, r.height() / 2 - m_nowPlayingIcon.height() / 2, 18 - r.width(), -r.height() / 2 + m_nowPlayingIcon.height() / 2  );
            painter->drawPixmap( npr, m_nowPlayingIcon );
            r.adjust( 22, 0, 0, 0 );
        }

        painter->setPen( opt.palette.text().color() );

        QRect ir = r.adjusted( 4, 0, -option.rect.width() + option.rect.height() - 8 + r.left(), 0 );

        QPixmap scover;
        if ( m_cache.contains( pixmap.cacheKey() ) )
        {
            scover = m_cache.value( pixmap.cacheKey() );
        }
        else
        {
            scover = pixmap.scaled( ir.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation );
            m_cache.insert( pixmap.cacheKey(), scover );
        }
        painter->drawPixmap( ir, scover );

        QFont boldFont = opt.font;
        boldFont.setBold( true );

        r.adjust( ir.width() + 12, 0, -12, 0 );
        painter->setFont( boldFont );
        QString text = painter->fontMetrics().elidedText( upperText, Qt::ElideRight, r.width() );
        painter->drawText( r.adjusted( 0, 1, 0, 0 ), text, m_topOption );


        painter->setFont( opt.font);
        text = painter->fontMetrics().elidedText( lowerText, Qt::ElideRight, r.width() );
        painter->drawText( r.adjusted( 0, 1, 0, 0 ), text, m_bottomOption );
    }
    painter->restore();
}


void
PlaylistItemDelegate::paintDetailed( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    TrackModelItem* item = m_model->itemFromIndex( m_model->mapToSource( index ) );
    Q_ASSERT( item );

    QStyleOptionViewItemV4 opt = option;
    prepareStyleOption( &opt, index, item );
    opt.text.clear();
    qApp->style()->drawControl( QStyle::CE_ItemViewItem, &opt, painter );

    if ( m_view->hoveredIndex().row() == index.row() && m_view->hoveredIndex().column() == index.column() &&
       ( index.column() == TrackModel::Artist || index.column() == TrackModel::Album ) )
    {
        opt.rect.setWidth( opt.rect.width() - 16 );
        QRect arrowRect( opt.rect.x() + opt.rect.width(), opt.rect.y() + 1, opt.rect.height() - 2, opt.rect.height() - 2 );

        if ( m_arrowIcon.height() != arrowRect.height() )
            m_arrowIcon = m_arrowIcon.scaled( arrowRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation );
        painter->drawPixmap( arrowRect, m_arrowIcon );
    }

    painter->save();

    if ( index.column() == TrackModel::Score )
    {
        QColor barColor( 167, 183, 211 ); // This matches the sidebar (sourcetreeview.cpp:672)
        if ( opt.state & QStyle::State_Selected )
            painter->setPen( opt.palette.brightText().color() );
        else
            painter->setPen( barColor );

        QRect r = opt.rect.adjusted( 3, 3, -6, -4 );
        painter->drawRect( r );

        QRect fillR = r;
        int fillerWidth = (int)( index.data().toFloat() * (float)fillR.width() );
        fillR.adjust( 0, 0, -( fillR.width() - fillerWidth ), 0 );

        if ( opt.state & QStyle::State_Selected )
            painter->setBrush( opt.palette.brightText().color() );
        else
            painter->setBrush( barColor );

        painter->drawRect( fillR );
    }
    else if ( item->isPlaying() )
    {
        {
            QRect r = opt.rect.adjusted( 3, 0, 0, 0 );

            // Paint Now Playing Speaker Icon
            if ( m_view->header()->visualIndex( index.column() ) == 0 )
            {
                r.adjust( 0, 0, 0, -3 );
                painter->drawPixmap( r.adjusted( 3, 1, 18 - r.width(), 1 ), m_nowPlayingIcon );
                r.adjust( 25, 0, 0, 3 );
            }

            painter->setPen( opt.palette.text().color() );
            QString text = painter->fontMetrics().elidedText( index.data().toString(), Qt::ElideRight, r.width() - 3 );
            painter->drawText( r.adjusted( 0, 1, 0, 0 ), text, m_centerOption );
        }
    }
    else
    {
        painter->setPen( opt.palette.text().color() );
        QString text = painter->fontMetrics().elidedText( index.data().toString(), Qt::ElideRight, opt.rect.width() - 3 );
        painter->drawText( opt.rect.adjusted( 3, 1, 0, 0 ), text, m_centerOption );
    }

    painter->restore();
}
