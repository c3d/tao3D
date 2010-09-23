// ****************************************************************************
//  history_playback.h                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//     HistoryPlayback implementation.
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "history_playback.h"
#include "repository.h"
#include "tao_utf8.h"

#define MAX_SPAN 100

namespace Tao {

HistoryPlayback::HistoryPlayback(QObject *parent)
// ----------------------------------------------------------------------------
//   Create object to manage navigation through document history
// ----------------------------------------------------------------------------
    : QObject(parent), repo(NULL), span(0), max_span(MAX_SPAN)
{
    reset();
}


HistoryPlayback::HistoryPlayback(QObject *parent, Repository *repo)
// ----------------------------------------------------------------------------
//   Create object to manage navigation through document history
// ----------------------------------------------------------------------------
    : QObject(parent), repo(repo), span(0), max_span(MAX_SPAN)
{
    reset();
}


void HistoryPlayback::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//   Set or change current repository
// ----------------------------------------------------------------------------
{
    this->repo = repo;
    connect(repo, SIGNAL(commitSuccess(QString,QString)),
            this, SLOT(newCommit(QString)));
    reset();
}


void HistoryPlayback::reset()
// ----------------------------------------------------------------------------
//   Reset current position to the head of the current branch
// ----------------------------------------------------------------------------
{
    if (!repo)
    {
        emit ready(false);
        return;
    }

    QString branch = +repo->branch();
    if (branch == "")
    {
        // We're on a branch with a detached head
        QList<Repository::Commit> last = repo->history("HEAD", 1);
        QString id = last[0].id;
        // If we are here as a result of our own checkout, do nothing
        if (span && (id == history[span - pos - 1].id))
            return;
        // Branch has been switched externally: reset playback data,
        // keeping commit id as a head identifier
        branch = id;
    }

    if (branch == head)
        return;

    head = branch;
    pos = 0;
    span = max_span;

    // 'span' is the number of commits we're allowed to navigate, relative to
    // the head of the branch at the time the HistoryPlayback object is
    // reset.
    history = repo->history(head, span);
    span = history.size();

    emit rangeChanged(min(), max());
    emit valueChanged(max() - pos);
    emit ready(true);
}


void HistoryPlayback::back()
// ----------------------------------------------------------------------------
//   Move one step backward in history (to older version)
// ----------------------------------------------------------------------------
{
    if (pos == span - 1)
        return;
    pos++;
    checkout(pos);
}


void HistoryPlayback::forward()
// ----------------------------------------------------------------------------
//   Move one step forward in history (to newer version)
// ----------------------------------------------------------------------------
{
    if (!pos)
        return;
    pos--;
    checkout(pos);
}


void HistoryPlayback::begin()
// ----------------------------------------------------------------------------
//   Move to begining of history (to oldest version)
// ----------------------------------------------------------------------------
{
    if (pos == span - 1)
        return;
    pos = span - 1;
    checkout(pos);
}


void HistoryPlayback::end()
// ----------------------------------------------------------------------------
//   Move to end of history (to newest version)
// ----------------------------------------------------------------------------
{
    if (pos == 0)
        return;
    pos = 0;
    checkout(pos);
}


void HistoryPlayback::newCommit(QString id)
// ----------------------------------------------------------------------------
//   Take into account the fact that branch head moved forward by 1 commit
// ----------------------------------------------------------------------------
//   This way, the begin position remains the same as doc changes
{
    span++;
    if (pos)
    {
        // pos == 0 means the latest commit. In this case we want to follow the
        // branch head and therefore do not modify pos.
        // When pos != 0 however, it refers to a specific, absolute position in
        // the current branch history, and thus we need to adjust pos when a
        // new commit comes in.
        pos++;
    }
    history.append(Repository::Commit(id));
    emit rangeChanged(min(), max());
    emit valueChanged(max()-pos);
}


void HistoryPlayback::setValue(int n)
// ----------------------------------------------------------------------------
//   Set history pointer to given position
// ----------------------------------------------------------------------------
{
    if (n < min() || n > max())
        return;
    if (pos == (max() - n))
        return;
    pos = max() - n;
    checkout(pos);
}


void HistoryPlayback::checkout(int n)
// ----------------------------------------------------------------------------
//   Checkout n-th previous version relative to branch head and emit signal
// ----------------------------------------------------------------------------
{
    QString what;
    if (n)
        what = history[span - n - 1].id;
    else
        what = head;
    repo->checkout(+what);
    emit valueChanged(max()-n);
}


void HistoryPlayback::debugPrintHistory()
// ----------------------------------------------------------------------------
//   Display history array to stderr
// ----------------------------------------------------------------------------
{
    QStringList list;
    foreach (Repository::Commit c, history)
        list << c.id;
    std::cerr << "history[0.." << history.size() << "] = "
              << +list.join(" ") << "\n";
}

}
