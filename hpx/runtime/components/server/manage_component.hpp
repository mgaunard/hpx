//  Copyright (c) 2007-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PP_IS_ITERATING

#if !defined(HPX_COMPONENTS_SERVER_MANAGE_COMPONENT_JUN_02_2008_0146PM)
#define HPX_COMPONENTS_SERVER_MANAGE_COMPONENT_JUN_02_2008_0146PM

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/iterate.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#include <hpx/exception.hpp>
#include <hpx/runtime/naming/address.hpp>
#include <hpx/runtime/naming/locality.hpp>
#include <hpx/runtime/naming/resolver_client.hpp>
#include <hpx/runtime/applier/applier.hpp>
#include <hpx/util/util.hpp>

#define BOOST_PP_ITERATION_PARAMS_1                                           \
    (3, (1, HPX_COMPONENT_CREATE_ARG_MAX,                                     \
    "hpx/runtime/components/server/manage_component.hpp"))                    \
    /**/

#include BOOST_PP_ITERATE()

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace components { namespace server
{
    ///////////////////////////////////////////////////////////////////////////
    /// Create arrays of components using their default constructor
    template <typename Component>
    naming::id_type create (std::size_t count, error_code& ec = throws)
    {
        if (0 == count) {
            HPX_THROWS_IF(ec, hpx::bad_parameter, 
                "create<Component>", "count shouldn't be zero");
            return naming::invalid_id;
        }

        Component* c = static_cast<Component*>(Component::create(count));
        naming::id_type gid = c->get_gid();
        if (gid) {
            if (&ec != &throws)
                ec = make_success_code();
            return gid;
        }

        delete c;
        HPX_THROWS_IF(ec, hpx::duplicate_component_address,
            "create<Component>", 
            "global id is already bound to a different "
            "component instance");
        return naming::invalid_id;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Component>
    void destroy(naming::id_type const& gid, error_code& ec = throws)
    {
        // retrieve the local address bound to the given global id
        applier::applier& appl = hpx::applier::get_applier();
        naming::address addr;
        if (!appl.get_agas_client().resolve(gid, addr)) 
        {
            HPX_OSSTREAM strm;
            strm << "global id " << gid << " is not bound to any "
                    "component instance";
            HPX_THROWS_IF(ec, hpx::unknown_component_address,
                "destroy<Component>", HPX_OSSTREAM_GETSTRING(strm));
            return;
        }

        // make sure this component is located here
        if (appl.here() != addr.locality_) 
        {
            // FIXME: should the component be re-bound ?
            HPX_OSSTREAM strm;
            strm << "global id " << gid << " is not bound to any local "
                    "component instance";
            HPX_THROWS_IF(ec, hpx::unknown_component_address,
                "destroy<Component>", HPX_OSSTREAM_GETSTRING(strm));
            return;
        }

        // make sure it's the correct component type
        components::component_type type = 
            components::get_component_type<typename Component::wrapped_type>();
        if (!types_are_compatible(type, addr.type_))
        {
            // FIXME: should the component be re-bound ?
            HPX_OSSTREAM strm;
            strm << "global id " << gid << " is not bound to a component "
                    "instance of type: " << get_component_type_name(type)
                 << " (it is bound to a " << get_component_type_name(addr.type_) 
                 << ")";
            HPX_THROWS_IF(ec, hpx::unknown_component_address,
                "destroy<Component>", HPX_OSSTREAM_GETSTRING(strm));
            return;
        }

        // delete the local instances
        Component::destroy(reinterpret_cast<Component*>(addr.address_));
        if (&ec != &throws)
            ec = make_success_code();
    }

}}}

#endif

///////////////////////////////////////////////////////////////////////////////
//  Preprocessor vertical repetition code
///////////////////////////////////////////////////////////////////////////////
#else // defined(BOOST_PP_IS_ITERATING)

#define N BOOST_PP_ITERATION()

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace components { namespace server
{
    ///////////////////////////////////////////////////////////////////////////
    /// Create single instances of a component using additional constructor 
    /// parameters
    template <typename Component, BOOST_PP_ENUM_PARAMS(N, typename T)>
    naming::id_type create_one(BOOST_PP_ENUM_BINARY_PARAMS(N, T, const& t))
    {
        Component* c = static_cast<Component*>(
            Component::create_one(BOOST_PP_ENUM_PARAMS(N, t)));
        naming::id_type gid = c->get_gid();
        if (gid) 
            return gid;

        delete c;
        HPX_THROW_EXCEPTION(hpx::duplicate_component_address,
            "create<Component>", 
            "global id is already bound to a different "
            "component instance");
        return naming::invalid_id;
    }

#undef N

}}}

#endif
