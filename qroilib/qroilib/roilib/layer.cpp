/*
 * layer.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
 *
 * This file is part of qroilib.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "layer.h"

#include "grouplayer.h"

#include "roimap.h"
#include "objectgroup.h"

namespace Qroilib {

Layer::Layer(TypeFlag type, const QString &name, int x, int y) :
    Object(LayerType),
    mName(name),
    mLayerType(type),
    mX(x),
    mY(y),
    mOpacity(1.0f),
    mVisible(true),
    mRoi(nullptr),
    mParentLayer(nullptr)
{
}

/**
 * Returns the effective opacity, which is the opacity multiplied by the
 * opacity of any parent layers.
 */
float Layer::effectiveOpacity() const
{
    auto opacity = mOpacity;
    const Layer *layer = this;
    while ((layer = layer->parentLayer()))
        opacity *= layer->opacity();
    return opacity;
}

/**
 * Returns whether this layer is hidden. A visible layer may still be hidden,
 * when one of its parent layers is not visible.
 */
bool Layer::isHidden() const
{
    const Layer *layer = this;
    while (layer && layer->isVisible())
        layer = layer->parentLayer();
    return layer;      // encountered an invisible layer
}

/**
 * Returns whether the given \a candidate is this layer or one of its parents.
 */
bool Layer::isParentOrSelf(const Layer *candidate) const
{
    const Layer *layer = this;
    while (layer != candidate && layer->parentLayer())
        layer = layer->parentLayer();
    return layer == candidate;
}

/**
 * Returns the depth of this layer in the hierarchy.
 */
int Layer::depth() const
{
    int d = 0;
    GroupLayer *p = mParentLayer;
    while (p) {
        ++d;
        p = p->parentLayer();
    }
    return d;
}

/**
 * Returns the index of this layer among its siblings.
 */
int Layer::siblingIndex() const
{
    if (mParentLayer)
        return mParentLayer->layers().indexOf(const_cast<Layer*>(this));
    if (mRoi)
        return mRoi->layers().indexOf(const_cast<Layer*>(this));
    return 0;
}

/**
 * Returns the list of siblings of this layer, including this layer.
 */
QList<Layer *> Layer::siblings() const
{
    if (mParentLayer)
        return mParentLayer->layers();
    if (mRoi)
        return mRoi->layers();

    return QList<Layer *>();
}

/**
 * Computes the total offset. which is the offset including the offset of all
 * parent layers.
 */
QPointF Layer::totalOffset() const
{
    auto offset = mOffset;
    const Layer *layer = this;
    while ((layer = layer->parentLayer()))
        offset += layer->offset();

    return offset;
}

/**
 * A helper function for initializing the members of the given instance to
 * those of this layer. Used by subclasses when cloning.
 *
 * Layer name, position and size are not cloned, since they are assumed to have
 * already been passed to the constructor. Also, roimap ownership is not cloned,
 * since the clone is not added to the roimap.
 *
 * \return the initialized clone (the same instance that was passed in)
 * \sa clone()
 */
Layer *Layer::initializeClone(Layer *clone) const
{
    clone->mOffset = mOffset;
    clone->mOpacity = mOpacity;
    clone->mVisible = mVisible;
    clone->setProperties(properties());
    return clone;
}

ObjectGroup *Layer::asObjectGroup()
{
    return isObjectGroup() ? static_cast<ObjectGroup*>(this) : nullptr;
}

GroupLayer *Layer::asGroupLayer()
{
    return isGroupLayer() ? static_cast<GroupLayer*>(this) : nullptr;
}

Layer *LayerIterator::next()
{
    if (!mCurrentLayer) {
        // Traverse to the first layer of the roimap
        if (mRoi && mSiblingIndex == -1 && mRoi->layerCount() > 0) {
            mCurrentLayer = mRoi->layerAt(0);
            mSiblingIndex = 0;
            return mCurrentLayer;
        }
        return nullptr;
    }

    const auto siblings = mCurrentLayer->siblings();
    int index = mSiblingIndex + 1;

    // Traverse to parent layer if last child
    if (index == siblings.size()) {
        mCurrentLayer = mCurrentLayer->parentLayer();
        mSiblingIndex = mCurrentLayer ? mCurrentLayer->siblingIndex() : -1;
        return mCurrentLayer;
    }

    // Traverse to next sibling
    Layer *layer = siblings.at(index);

    // If next layer is a group, traverse to its first child
    while (layer->isGroupLayer()) {
        auto groupLayer = static_cast<GroupLayer*>(layer);
        if (groupLayer->layerCount() > 0) {
            index = 0;
            layer = groupLayer->layerAt(0);
        } else {
            break;
        }
    }

    mCurrentLayer = layer;
    mSiblingIndex = index;

    return layer;
}

Layer *LayerIterator::previous()
{
    Layer *layer = mCurrentLayer;
    int index = mSiblingIndex - 1;

    if (!layer) {
        // Traverse to the last layer of the roimap if at the end
        if (mRoi && index < mRoi->layerCount() && mRoi->layerCount() > 0) {
            layer = mRoi->layerAt(index);
        } else {
            return nullptr;
        }
    } else {
        // Traverse down to last child if applicable
        if (layer->isGroupLayer()) {
            auto groupLayer = static_cast<GroupLayer*>(layer);
            if (groupLayer->layerCount() > 0) {
                mSiblingIndex = groupLayer->layerCount() - 1;
                mCurrentLayer = groupLayer->layerAt(mSiblingIndex);
                return mCurrentLayer;
            }
        }

        // Traverse to previous sibling (possibly of a parent)
        do {
            if (index >= 0) {
                const auto siblings = layer->siblings();
                layer = siblings.at(index);
                break;
            }

            layer = layer->parentLayer();
            if (layer)
                index = layer->siblingIndex() - 1;
        } while (layer);
    }

    mCurrentLayer = layer;
    mSiblingIndex = index;

    return layer;
}

void LayerIterator::toFront()
{
    mCurrentLayer = nullptr;
    mSiblingIndex = -1;
}

void LayerIterator::toBack()
{
    mCurrentLayer = nullptr;
    mSiblingIndex = mRoi ? mRoi->layerCount() : -1;
}


/**
 * Returns the global layer index for the given \a layer. Obtained by iterating
 * the layer's roimap while incrementing the index until layer is found.
 */
int globalIndex(Layer *layer)
{
    if (!layer)
        return -1;

    LayerIterator counter(layer->roimap());
    int index = 0;
    while (counter.next() && counter.currentLayer() != layer)
        ++index;

    return index;
}

/**
 * Returns the layer at the given global \a index.
 *
 * \sa globalIndex()
 */
Layer *layerAtGlobalIndex(const RoiMap *roimap, int index)
{
    LayerIterator counter(roimap);
    while (counter.next() && index > 0)
        --index;

    return counter.currentLayer();
}

} // namespace Qroilib
