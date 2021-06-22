#pragma once

#include <locale>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cassert>

// See below for licence.
// Credits to https://github.com/spacemoose/ostream_indenter
// for creating this library

class indent_facet : public std::codecvt<char, char, std::mbstate_t> {
public:
	explicit indent_facet( int indent_level, size_t ref = 0)
		: std::codecvt<char, char, std::mbstate_t>(ref), m_indentation_level(indent_level) {}
	typedef std::codecvt_base::result result;
	typedef std::codecvt<char, char, std::mbstate_t> parent;
	typedef parent::intern_type intern_type;
	typedef parent::extern_type extern_type;
	typedef parent::state_type  state_type;

	int &state(state_type &s) const { return *reinterpret_cast<int *>(&s); }

protected:
	virtual result do_out(state_type &need_indentation,
		const intern_type *from, const intern_type *from_end, const intern_type *&from_next,
		extern_type *to, extern_type *to_end, extern_type *&to_next
		) const override;

	// Override so the do_out() virtual function is called.
	virtual bool do_always_noconv() const throw() override {
		return m_indentation_level==0;
	}
	int m_indentation_level = 0;

};

inline indent_facet::result indent_facet::do_out(state_type &need_indentation,
	const intern_type *from, const intern_type *from_end, const intern_type *&from_next,
	extern_type *to, extern_type *to_end, extern_type *&to_next
	) const
{
	result res = std::codecvt_base::noconv;
	for (; (from < from_end) && (to < to_end); ++from, ++to) {
		// 0 indicates that the last character seen was a newline.
		// thus we will print a tab before it. Ignore it the next
		// character is also a newline
		if ((state(need_indentation) == 0) && (*from != '\n')) {
			res = std::codecvt_base::ok;
			state(need_indentation) = 1;
			for(int i=0; i<m_indentation_level; ++i){
				*to = '\t'; ++to;
			}
			if (to == to_end) {
				res = std::codecvt_base::partial;
				break;
			}
		}
		*to = *from; // Copy the next character.

		// If the character copied was a '\n' mark that state
		if (*from == '\n') {
			state(need_indentation) = 0;
		}
	}

	if (from != from_end) {
		res = std::codecvt_base::partial;
	}
	from_next = from;
	to_next = to;

	return res;
};



/// I hate the way I solved this, but I can't think of a better way
/// around the problem.  I even asked stackoverflow for help:
///
///   http://stackoverflow.com/questions/32480237/apply-a-facet-to-all-stream-output-use-custom-string-manipulators
///
///
namespace  indent_manip{

static const int index = std::ios_base::xalloc();

inline static std::ostream & push(std::ostream& os)
{
	auto ilevel = ++os.iword(index);
	os.imbue(std::locale(os.getloc(), new indent_facet(ilevel)));
	return os;
}

inline std::ostream& pop(std::ostream& os)
{
	auto ilevel = (os.iword(index)>0) ? --os.iword(index) : 0;
	os.imbue(std::locale(os.getloc(), new indent_facet(ilevel)));
	return os;
}

/// Clears the ostream indentation set, but NOT the raii_guard.
inline std::ostream& clear(std::ostream& os)
{
	os.iword(index) = 0;
	os.imbue(std::locale(os.getloc(), new indent_facet(0)));
	return os;
}



/// Provides a RAII guard around your manipulation.
class raii_guard
{
public:
	raii_guard(std::ostream& os):
		oref(os),
		start_level(os.iword(index))
	{}

	~raii_guard()
	{
		reset();
	}

	/// Resets the streams indentation level to the point itw as at
	/// when the guard was created.
	void reset()
	{
		oref.iword(index) = start_level;
		oref.imbue(std::locale(oref.getloc(), new indent_facet(start_level)));
	}

private:
	std::ostream& oref;
	int start_level;
};

}

// This library is licensed under the lgpl.  Roughly speaking this means
// you can use it a proprietary project as long as you

//    1.  Credit the library.
//    2.  Share any improvement you make to the library.  You can do that by
//        contributing a pull request to the github site.

// A good plain language description of the requirements of the LGPL is found
// here:
// http://eigen.tuxfamily.org/index.php?title=Licensing_FAQ&oldid=1117#General_licensing_questions


// GNU LESSER GENERAL PUBLIC LICENSE
//                        Version 3, 29 June 2007

//  Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
//  Everyone is permitted to copy and distribute verbatim copies
//  of this license document, but changing it is not allowed.


//   This version of the GNU Lesser General Public License incorporates
// the terms and conditions of version 3 of the GNU General Public
// License, supplemented by the additional permissions listed below.

//   0. Additional Definitions.

//   As used herein, "this License" refers to version 3 of the GNU Lesser
// General Public License, and the "GNU GPL" refers to version 3 of the GNU
// General Public License.

//   "The Library" refers to a covered work governed by this License,
// other than an Application or a Combined Work as defined below.

//   An "Application" is any work that makes use of an interface provided
// by the Library, but which is not otherwise based on the Library.
// Defining a subclass of a class defined by the Library is deemed a mode
// of using an interface provided by the Library.

//   A "Combined Work" is a work produced by combining or linking an
// Application with the Library.  The particular version of the Library
// with which the Combined Work was made is also called the "Linked
// Version".

//   The "Minimal Corresponding Source" for a Combined Work means the
// Corresponding Source for the Combined Work, excluding any source code
// for portions of the Combined Work that, considered in isolation, are
// based on the Application, and not on the Linked Version.

//   The "Corresponding Application Code" for a Combined Work means the
// object code and/or source code for the Application, including any data
// and utility programs needed for reproducing the Combined Work from the
// Application, but excluding the System Libraries of the Combined Work.

//   1. Exception to Section 3 of the GNU GPL.

//   You may convey a covered work under sections 3 and 4 of this License
// without being bound by section 3 of the GNU GPL.

//   2. Conveying Modified Versions.

//   If you modify a copy of the Library, and, in your modifications, a
// facility refers to a function or data to be supplied by an Application
// that uses the facility (other than as an argument passed when the
// facility is invoked), then you may convey a copy of the modified
// version:

//    a) under this License, provided that you make a good faith effort to
//    ensure that, in the event an Application does not supply the
//    function or data, the facility still operates, and performs
//    whatever part of its purpose remains meaningful, or

//    b) under the GNU GPL, with none of the additional permissions of
//    this License applicable to that copy.

//   3. Object Code Incorporating Material from Library Header Files.

//   The object code form of an Application may incorporate material from
// a header file that is part of the Library.  You may convey such object
// code under terms of your choice, provided that, if the incorporated
// material is not limited to numerical parameters, data structure
// layouts and accessors, or small macros, inline functions and templates
// (ten or fewer lines in length), you do both of the following:

//    a) Give prominent notice with each copy of the object code that the
//    Library is used in it and that the Library and its use are
//    covered by this License.

//    b) Accompany the object code with a copy of the GNU GPL and this license
//    document.

//   4. Combined Works.

//   You may convey a Combined Work under terms of your choice that,
// taken together, effectively do not restrict modification of the
// portions of the Library contained in the Combined Work and reverse
// engineering for debugging such modifications, if you also do each of
// the following:

//    a) Give prominent notice with each copy of the Combined Work that
//    the Library is used in it and that the Library and its use are
//    covered by this License.

//    b) Accompany the Combined Work with a copy of the GNU GPL and this license
//    document.

//    c) For a Combined Work that displays copyright notices during
//    execution, include the copyright notice for the Library among
//    these notices, as well as a reference directing the user to the
//    copies of the GNU GPL and this license document.

//    d) Do one of the following:

//        0) Convey the Minimal Corresponding Source under the terms of this
//        License, and the Corresponding Application Code in a form
//        suitable for, and under terms that permit, the user to
//        recombine or relink the Application with a modified version of
//        the Linked Version to produce a modified Combined Work, in the
//        manner specified by section 6 of the GNU GPL for conveying
//        Corresponding Source.

//        1) Use a suitable shared library mechanism for linking with the
//        Library.  A suitable mechanism is one that (a) uses at run time
//        a copy of the Library already present on the user's computer
//        system, and (b) will operate properly with a modified version
//        of the Library that is interface-compatible with the Linked
//        Version.

//    e) Provide Installation Information, but only if you would otherwise
//    be required to provide such information under section 6 of the
//    GNU GPL, and only to the extent that such information is
//    necessary to install and execute a modified version of the
//    Combined Work produced by recombining or relinking the
//    Application with a modified version of the Linked Version. (If
//    you use option 4d0, the Installation Information must accompany
//    the Minimal Corresponding Source and Corresponding Application
//    Code. If you use option 4d1, you must provide the Installation
//    Information in the manner specified by section 6 of the GNU GPL
//    for conveying Corresponding Source.)

//   5. Combined Libraries.

//   You may place library facilities that are a work based on the
// Library side by side in a single library together with other library
// facilities that are not Applications and are not covered by this
// License, and convey such a combined library under terms of your
// choice, if you do both of the following:

//    a) Accompany the combined library with a copy of the same work based
//    on the Library, uncombined with any other library facilities,
//    conveyed under the terms of this License.

//    b) Give prominent notice with the combined library that part of it
//    is a work based on the Library, and explaining where to find the
//    accompanying uncombined form of the same work.

//   6. Revised Versions of the GNU Lesser General Public License.

//   The Free Software Foundation may publish revised and/or new versions
// of the GNU Lesser General Public License from time to time. Such new
// versions will be similar in spirit to the present version, but may
// differ in detail to address new problems or concerns.

//   Each version is given a distinguishing version number. If the
// Library as you received it specifies that a certain numbered version
// of the GNU Lesser General Public License "or any later version"
// applies to it, you have the option of following the terms and
// conditions either of that published version or of any later version
// published by the Free Software Foundation. If the Library as you
// received it does not specify a version number of the GNU Lesser
// General Public License, you may choose any version of the GNU Lesser
// General Public License ever published by the Free Software Foundation.

//   If the Library as you received it specifies that a proxy can decide
// whether future versions of the GNU Lesser General Public License shall
// apply, that proxy's public statement of acceptance of any version is
// permanent authorization for you to choose that version for the
// Library.
