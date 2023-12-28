// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  UnlabeledValueArg.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno.
 *  Copyright (c) 2017, Google LLC
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef TCLAP_UNLABELED_VALUE_ARG_H
#define TCLAP_UNLABELED_VALUE_ARG_H

#include <tclap/OptionalUnlabeledTracker.h>
#include <tclap/ValueArg.h>

#include <list>
#include <string>
#include <vector>

namespace TCLAP {

/**
 * The basic unlabeled argument that parses a value.
 * This is a template class, which means the type T defines the type
 * that a given object will attempt to parse when an UnlabeledValueArg
 * is reached in the list of args that the CmdLine iterates over.
 */
template <class T>
class UnlabeledValueArg : public ValueArg<T> {
    // If compiler has two stage name lookup (as gcc >= 3.4 does)
    // this is required to prevent undef. symbols
    using ValueArg<T>::_ignoreable;
    using ValueArg<T>::_hasBlanks;
    using ValueArg<T>::_extractValue;
    using ValueArg<T>::_typeDesc;
    using ValueArg<T>::_name;
    using ValueArg<T>::_description;
    using ValueArg<T>::_alreadySet;
    using ValueArg<T>::_setBy;
    using ValueArg<T>::toString;

public:
    /**
     * UnlabeledValueArg constructor.
     * \param name - A one word name for the argument.  Note that this is used
     * for
     * identification, not as a long flag.
     * \param desc - A description of what the argument is for or
     * does.
     * \param req - Whether the argument is required on the command
     * line.
     * \param value - The default value assigned to this argument if it
     * is not present on the command line.
     * \param typeDesc - A short, human readable description of the
     * type that this object expects.  This is used in the generation
     * of the USAGE statement.  The goal is to be helpful to the end user
     * of the program.
     * \param ignoreable - Allows you to specify that this argument can be
     * ignored if the '--' flag is set.  This defaults to false (cannot
     * be ignored) and should  generally stay that way unless you have
     * some special need for certain arguments to be ignored.
     * \param v - Optional Visitor.  You should leave this blank unless
     * you have a very good reason.
     */
    UnlabeledValueArg(const std::string &name, const std::string &desc,
                      bool req, T value, const std::string &typeDesc,
                      bool ignoreable = false, Visitor *v = NULL);

    /**
     * UnlabeledValueArg constructor.
     * \param name - A one word name for the argument.  Note that this is used
     * for
     * identification, not as a long flag.
     * \param desc - A description of what the argument is for or
     * does.
     * \param req - Whether the argument is required on the command
     * line.
     * \param value - The default value assigned to this argument if it
     * is not present on the command line.
     * \param typeDesc - A short, human readable description of the
     * type that this object expects.  This is used in the generation
     * of the USAGE statement.  The goal is to be helpful to the end user
     * of the program.
     * \param parser - A CmdLine parser object to add this Arg to
     * \param ignoreable - Allows you to specify that this argument can be
     * ignored if the '--' flag is set.  This defaults to false (cannot
     * be ignored) and should  generally stay that way unless you have
     * some special need for certain arguments to be ignored.
     * \param v - Optional Visitor.  You should leave this blank unless
     * you have a very good reason.
     */
    UnlabeledValueArg(const std::string &name, const std::string &desc,
                      bool req, T value, const std::string &typeDesc,
                      CmdLineInterface &parser, bool ignoreable = false,
                      Visitor *v = NULL);

    /**
     * UnlabeledValueArg constructor.
     * \param name - A one word name for the argument.  Note that this is used
     * for
     * identification, not as a long flag.
     * \param desc - A description of what the argument is for or
     * does.
     * \param req - Whether the argument is required on the command
     * line.
     * \param value - The default value assigned to this argument if it
     * is not present on the command line.
     * \param constraint - A pointer to a Constraint object used
     * to constrain this Arg.
     * \param ignoreable - Allows you to specify that this argument can be
     * ignored if the '--' flag is set.  This defaults to false (cannot
     * be ignored) and should  generally stay that way unless you have
     * some special need for certain arguments to be ignored.
     * \param v - Optional Visitor.  You should leave this blank unless
     * you have a very good reason.
     */
    UnlabeledValueArg(const std::string &name, const std::string &desc,
                      bool req, T value, Constraint<T> *constraint,
                      bool ignoreable = false, Visitor *v = NULL);

    /**
     * UnlabeledValueArg constructor.
     * \param name - A one word name for the argument.  Note that this is used
     * for
     * identification, not as a long flag.
     * \param desc - A description of what the argument is for or
     * does.
     * \param req - Whether the argument is required on the command
     * line.
     * \param value - The default value assigned to this argument if it
     * is not present on the command line.
     * \param constraint - A pointer to a Constraint object used
     * to constrain this Arg.
     * \param parser - A CmdLine parser object to add this Arg to
     * \param ignoreable - Allows you to specify that this argument can be
     * ignored if the '--' flag is set.  This defaults to false (cannot
     * be ignored) and should  generally stay that way unless you have
     * some special need for certain arguments to be ignored.
     * \param v - Optional Visitor.  You should leave this blank unless
     * you have a very good reason.
     */
    UnlabeledValueArg(const std::string &name, const std::string &desc,
                      bool req, T value, Constraint<T> *constraint,
                      CmdLineInterface &parser, bool ignoreable = false,
                      Visitor *v = NULL);

    /**
     * Handles the processing of the argument.
     * This re-implements the Arg version of this method to set the
     * _value of the argument appropriately.  Handling specific to
     * unlabeled arguments.
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings.
     */
    virtual bool processArg(int *i, std::vector<std::string> &args);

    /**
     * Overrides shortID for specific behavior.
     */
    virtual std::string shortID(const std::string &) const {
        return Arg::getName();
    }

    /**
     * Overrides longID for specific behavior.
     */
    virtual std::string longID(const std::string &) const {
        return Arg::getName() + " <" + _typeDesc + ">";
    }

    /**
     * Overrides operator== for specific behavior.
     */
    virtual bool operator==(const Arg &a) const;

    /**
     * Instead of pushing to the front of list, push to the back.
     * \param argList - The list to add this to.
     */
    virtual void addToList(std::list<Arg *> &argList) const;

    virtual bool hasLabel() const { return false; }
};

/**
 * Constructor implementation.
 */
template <class T>
UnlabeledValueArg<T>::UnlabeledValueArg(const std::string &name,
                                        const std::string &desc, bool req,
                                        T val, const std::string &typeDesc,
                                        bool ignoreable, Visitor *v)
    : ValueArg<T>("", name, desc, req, val, typeDesc, v) {
    _ignoreable = ignoreable;

    OptionalUnlabeledTracker::check(req, toString());
}

template <class T>
UnlabeledValueArg<T>::UnlabeledValueArg(const std::string &name,
                                        const std::string &desc, bool req,
                                        T val, const std::string &typeDesc,
                                        CmdLineInterface &parser,
                                        bool ignoreable, Visitor *v)
    : ValueArg<T>("", name, desc, req, val, typeDesc, v) {
    _ignoreable = ignoreable;
    OptionalUnlabeledTracker::check(req, toString());
    parser.add(this);
}

/**
 * Constructor implementation.
 */
template <class T>
UnlabeledValueArg<T>::UnlabeledValueArg(const std::string &name,
                                        const std::string &desc, bool req,
                                        T val, Constraint<T> *constraint,
                                        bool ignoreable, Visitor *v)
    : ValueArg<T>("", name, desc, req, val, constraint, v) {
    _ignoreable = ignoreable;
    OptionalUnlabeledTracker::check(req, toString());
}

template <class T>
UnlabeledValueArg<T>::UnlabeledValueArg(const std::string &name,
                                        const std::string &desc, bool req,
                                        T val, Constraint<T> *constraint,
                                        CmdLineInterface &parser,
                                        bool ignoreable, Visitor *v)
    : ValueArg<T>("", name, desc, req, val, constraint, v) {
    _ignoreable = ignoreable;
    OptionalUnlabeledTracker::check(req, toString());
    parser.add(this);
}

/**
 * Implementation of processArg().
 */
template <class T>
bool UnlabeledValueArg<T>::processArg(int *i, std::vector<std::string> &args) {
    if (_alreadySet) return false;

    if (_hasBlanks(args[*i])) return false;

    // never ignore an unlabeled arg

    _extractValue(args[*i]);
    _alreadySet = true;
    _setBy = args[*i];
    this->_checkWithVisitor();
    return true;
}

/**
 * Overriding operator== for specific behavior.
 */
template <class T>
bool UnlabeledValueArg<T>::operator==(const Arg &a) const {
    if (_name == a.getName() || _description == a.getDescription())
        return true;
    else
        return false;
}

template <class T>
void UnlabeledValueArg<T>::addToList(std::list<Arg *> &argList) const {
    argList.push_back(const_cast<Arg *>(static_cast<const Arg *const>(this)));
}
}  // namespace TCLAP

#endif  // TCLAP_UNLABELED_VALUE_ARG_H
