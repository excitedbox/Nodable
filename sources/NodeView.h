#pragma once

#include "Nodable.h" // for constants and forward declarations
#include "View.h"    // base class
#include <imgui/imgui.h>   // for ImVec2
#include <string>
#include <map>

#define NODE_VIEW_DEFAULT_SIZE ImVec2(120.0f, 120.0f)

namespace Nodable{

	/* We use this enum to identify all GUI detail modes */
	enum DrawDetail_
	{
		DrawDetail_Simple   = 0,                 // node and links.
		DrawDetail_Advanced = 1,                 // node, links and input/output names.
		DrawDetail_Complex  = 2,                 // node, links, input/output names and types.
		DrawDetail_Default  = DrawDetail_Simple
	};

	class NodeView : public View
	{
	private:
		/* Update function that takes a specific delta time (can be hacked by sending a custom value) */
		bool              update(float _deltaTime);

	public:
		COMPONENT_CONSTRUCTOR(NodeView);

		/* Draw the view at its position into the current window
		   Returns true if nod has been edited, false either */
		bool              draw                ()override;

		/* Should be called once per frame to update the view */
		bool              update              ()override;		

		void updateInputConnectedNodes(Nodable::Entity* node, float deltaTime);

		/* Get top-left corner vector position */
		ImVec2            getRoundedPosition         ()const;

		/* Get the connector position of the specified member (by name) for its Connection_ side (In or Out ONLY !) */
		ImVec2            getMemberConnectorPosition(const std::string& /*_name*/, Connection_ /*_connection*/)const;

		/* Set a new position (top-left corner) vector to this view */ 
		void              setPosition         (ImVec2);

		/* Apply a translation vector to the view's position */
		void              translate           (ImVec2);

		/* Arrange input nodes recursively while keeping this node position unchanged */
		void              arrangeRecursively  ();
		
		/* Arrange input nodes recursively while keeping the nodeView at the position vector _position */
		static void       ArrangeRecursively  (NodeView* /*_nodeView*/, ImVec2 _position = ImVec2(700.0f, 350.0f));		

		/* Set the _nodeView as selected. 
		Only a single view can be selected at the same time */
		static void       SetSelected         (NodeView*);

		/* Return a pointer to the selected view or nullptr if no view are selected */
		static NodeView*  GetSelected         ();

		/* Return a pointer to the dragged member or nullptr if no member is dragged */
		static Member*    GetDraggedByMouseMember() { return s_draggedByMouseMember; }

		/* Return a pointer to the hovered member or nullptr if no member is dragged */
		static Member*    GetHoveredByMouseMember() { return s_hoveredByMouseMember; }

		static void       ResetDraggedByMouseMember() { s_draggedByMouseMember = nullptr; }

		/* Return true if _nodeView is selected */
		static bool       IsSelected          (NodeView*);

		/* Set the _nodeView ad the current dragged view.
		Only a single view can be dragged at the same time. */
		static void       SetDragged          (NodeView*);

		static bool		  IsANodeDragged();

		/* Return a pointer to the dragged view or nullptr if no view are dragged */
		static NodeView*  GetDragged          ();

		static DrawDetail_ s_drawDetail;  // global draw detail (check DrawDetail_ enum)

	private:
		/*	Draw a Node Member at cursor position.
			Returns true if Member's value has been modified, false either */
		bool			  drawMember(Member* _v);

		void drawMemberConnector(ImVec2& connectorPos, Nodable::Member* _member, ImDrawList* draw_list);

		ImVec2          position            = ImVec2(500.0f, -1.0f);    // center position vector
		ImVec2          size                = NODE_VIEW_DEFAULT_SIZE;  // size of the window
		float           opacity             = 1.0f;                   // global transparency of this view                 
		bool            collapsed           = true;
		bool            pinned              = false;                  // false: follow its outputs.
		float           borderRadius        = 5.0f;
		ImColor         borderColorSelected = ImColor(1.0f, 1.0f, 1.0f);
		std::map<std::string, float> connectorOffsetPositionsY;
		static NodeView* s_selected; // pointer to the currently selected NodeView.
		static NodeView* s_dragged;	 // pointer to the currently dragged NodeView.	
		static Member*   s_draggedByMouseMember;
		static Member*   s_hoveredByMouseMember;
	};
}
