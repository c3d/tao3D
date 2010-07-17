// ****************************************************************************
//  layout.cpp                                                      Tao project
// ****************************************************************************
// 
//   File Description:
// 
//     Drawing object that is used to lay out objects in 2D or 3D space
//     Layouts are used to represent 2D pages, 3D spaces (Z ordering)
//     and text made of a sequence of glyphs
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
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "layout.h"
#include "gl_keepers.h"
#include "attributes.h"
#include "gl2ps.h"

TAO_BEGIN

int   Layout::polygonOffset   = 0;
scale Layout::factorBase      = 0;
scale Layout::factorIncrement = -0.005; // Experimental value
scale Layout::unitBase        = 0;
scale Layout::unitIncrement   = -1;


LayoutState::LayoutState()
// ----------------------------------------------------------------------------
//   Default state for all layouts
// ----------------------------------------------------------------------------
    : offset(),
      font(qApp->font()),
      alongX(), alongY(), alongZ(),
      left(0), right(0), top(0), bottom(0),
      lineWidth(1.0),
      lineColor(0,0,0,1),       // Black
      fillColor(0,0,0,0),       // Transparent black
      fillTexture(0), wrapS(false), wrapT(false), printing(false),
      planarRotation(0), planarScale(1),
      rotationId(0), translationId(0), scaleId(0)
{}


LayoutState::LayoutState(const LayoutState &o)
// ----------------------------------------------------------------------------
//   Copy state (may be used between layouts)
// ----------------------------------------------------------------------------
      : offset(o.offset),
        font(o.font),
        alongX(o.alongX), alongY(o.alongY), alongZ(o.alongZ),
        left(o.left), right(o.right),
        top(o.top), bottom(o.bottom),
        lineWidth(o.lineWidth),
        lineColor(o.lineColor),
        fillColor(o.fillColor),
        fillTexture(o.fillTexture),
        wrapS(o.wrapS),
        wrapT(o.wrapT),
        printing(o.printing),
        planarRotation(o.planarRotation),
        planarScale(o.planarScale),
        rotationId(o.rotationId),
        translationId(o.translationId),
        scaleId(o.scaleId)
{}



void LayoutState::Clear()
// ----------------------------------------------------------------------------
//   Reset default state for a layout
// ----------------------------------------------------------------------------
{
    offset.Set(0,0,0);
    font = qApp->font();

    alongX = alongY = alongZ = Justification();
    lineColor.Set(0,0,0,0); // Transparent black
    fillColor.Set(0,0,0,1); // Black
    fillTexture = 0;
}


Layout::Layout(Widget *widget)
// ----------------------------------------------------------------------------
//    Create an empty layout
// ----------------------------------------------------------------------------
    : Drawing(), LayoutState(), id(0),
      hasPixelBlur(false), hasMatrix(false), has3D(false),
      hasAttributes(false), hasTextureMatrix(false),
      isSelection(false),
      items(), display(widget)
{}


Layout::Layout(const Layout &o)
// ----------------------------------------------------------------------------
//   Copy constructor
// ----------------------------------------------------------------------------
    : Drawing(o), LayoutState(o), id(0),
      hasPixelBlur(o.hasPixelBlur), hasMatrix(false), has3D(o.has3D),
      hasAttributes(false), hasTextureMatrix(false),
      isSelection(o.isSelection),
      items(), display(o.display)
{}


Layout::~Layout()
// ----------------------------------------------------------------------------
//   Destroy a layout
// ----------------------------------------------------------------------------
{
    Clear();
}


Layout *Layout::AddChild(uint childId)
// ----------------------------------------------------------------------------
//   Add a new layout as a child of this one
// ----------------------------------------------------------------------------
{
    Layout *result = NewChild();
    Add(result);
    result->id = childId;
    return result;
}


void Layout::Clear()
// ----------------------------------------------------------------------------
//   Reset the layout to the initial setup
// ----------------------------------------------------------------------------
{
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
        delete *i;
    items.clear();

    // Initial state has no rotation or attribute changes
    hasPixelBlur = false;
    hasMatrix = false;
    has3D = false;
    hasAttributes = false;

    LayoutState::Clear();
}


// The bit values we save for a layout
static const GLbitfield GL_LAYOUT_BITS = GL_LINE_BIT | GL_TEXTURE_BIT;


void Layout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::LocalSave<Point3> save(offset, offset);
    GLAllStateKeeper glSave(hasAttributes?GL_LAYOUT_BITS:0,
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);

    // Display all items
    PushLayout(where);
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->Draw(this);
    }
    PopLayout(where);
}


void Layout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection for the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::LocalSave<Point3> save(offset, offset);
    GLAllStateKeeper glSave(hasAttributes?GL_LAYOUT_BITS:0,
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);

    PushLayout(where);
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->DrawSelection(this);
    }
    PopLayout(where);
}


void Layout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify the elements of the layout for OpenGL selection
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::LocalSave<Point3> save(offset, offset);
    GLAllStateKeeper glSave(hasAttributes?GL_LAYOUT_BITS:0,
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);


    PushLayout(where);
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->Identify(this);
    }
    PopLayout(where);
}


Vector3 Layout::Offset()
// ----------------------------------------------------------------------------
//   Return the offset at which the layout expects us to draw
// ----------------------------------------------------------------------------
{
    return offset;
}


Box3 Layout::Bounds(Layout *layout)
// ----------------------------------------------------------------------------
//   Compute the bounding box as the union of all item bounds
// ----------------------------------------------------------------------------
{
    Box3 result;
    layout_items::iterator i;
    Inherit(layout);
    if (items.size() == 0)
        result |= Point3();
    else
        for (i = items.begin(); i != items.end(); i++)
            result |= (*i)->Bounds(this);
    return result;
}


Box3 Layout::Space(Layout *layout)
// ----------------------------------------------------------------------------
//   Compute the required space as the union of all item bounds
// ----------------------------------------------------------------------------
{
    Box3 result;
    layout_items::iterator i;
    Inherit(layout);
    if (items.size() == 0)
        result |= Point3();
    else
        for (i = items.begin(); i != items.end(); i++)
            result |= (*i)->Space(this);
    return result;
}


void Layout::Add(Drawing *d)
// ----------------------------------------------------------------------------
//   Add a drawing to the items, return true if item fits in layout
// ----------------------------------------------------------------------------
{
    items.push_back(d);
}


void Layout::PolygonOffset()
// ----------------------------------------------------------------------------
//   Compute a polygon offset for the next shape being drawn
// ----------------------------------------------------------------------------
{
    int offset = polygonOffset++;
    glPolygonOffset (factorBase + offset * factorIncrement,
                     unitBase + offset * unitIncrement);
    if (printing)
        gl2psEnable(GL2PS_POLYGON_OFFSET_FILL);
}


uint Layout::ChildrenSelected()
// ----------------------------------------------------------------------------
//   The sum of chilren selections
// ----------------------------------------------------------------------------
{
    uint result = 0;
    Layout *l;
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
        if ((l = dynamic_cast<Layout*>(*i)))
            result += l->Selected();
    return result;
}


uint Layout::Selected()
// ----------------------------------------------------------------------------
//   Selection state of this layout plus the sum of chilren selections
// ----------------------------------------------------------------------------
{
    uint selected = Display()->selected(id);
    selected &= Widget::SELECTION_MASK;
    return selected + ChildrenSelected();
}


LayoutState & Layout::operator=(const LayoutState &o)
// ----------------------------------------------------------------------------
//   Restore a previously saved state
// ----------------------------------------------------------------------------
{
    *((LayoutState *) this) = o;
    return *this;
}


void Layout::Inherit(Layout *where)
// ----------------------------------------------------------------------------
//   Inherit state from some other layout
// ----------------------------------------------------------------------------
{
    if (!where)
        return;

    // Add offset of parent to the one we have
    offset = where->Offset();

    // Inherit color and other parameters as initial values
    // Note that these may really impact what gets rendered,
    // e.g. transparent colors may cause shapes to be drawn or not
    font            = where->font;
    alongX          = where->alongX;
    alongY          = where->alongY;
    alongZ          = where->alongZ;
    left            = where->left;
    right           = where->right;
    top             = where->top;
    bottom          = where->bottom;
    lineWidth       = where->lineWidth;
    lineColor       = where->lineColor;
    fillColor       = where->fillColor;
    fillTexture     = where->fillTexture;
    wrapS           = where->wrapS;
    wrapT           = where->wrapT;
    printing        = where->printing;
    planarRotation  = where->planarRotation;
    planarScale     = where->planarScale;
    has3D           = where->has3D;
    hasPixelBlur    = where->hasPixelBlur;
}


void Layout::PushLayout(Layout *where)
// ----------------------------------------------------------------------------
//   Save away information required to maintain selection hierarchy
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    widget->selectionContainerPush();
}


void Layout::PopLayout(Layout *where)
// ----------------------------------------------------------------------------
//   Restore information required to maintain selection hierarchy
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    widget->selectionContainerPop();
}

TAO_END
