package com.leidos.inspection;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.baeldung.openapi.model.InspectionRequest;
import com.baeldung.openapi.model.InspectionStatus;
import com.baeldung.openapi.model.InspectionStatus.StatusEnum;

import org.junit.jupiter.api.BeforeEach;

/**
 * Test Class to test {@link InspectionActions} class.
 * 
 * @author Paul Bourelly
 */
public class InspectionActionsTest {

    private InspectionActions inspectionActions;

    /**
     * Init to run before each test
     */
    @BeforeEach
    public void init() {
        // Initialize Inspection Action Bean
        inspectionActions = new InspectionActions();
    }

    /**
     * Test case to test {@link InspectionActions#getInspectionStatus(String)} and
     * {@link InspectionActions#requestInspectionAction(InspectionRequest)} for
     * different possible inputs.
     */
    @Test
    public void requestInspectionTest() {
        // Returns null when provided null vehicleId
        assertNull(inspectionActions.getInspectionStatus(null));

        // requestInspectionAction does not throw exceptions for null parameters
        inspectionActions.requestInspectionAction(null);

        // Populate Inspection actions with inspection requests
        InspectionRequest req1 = new InspectionRequest();
        req1.setVehicleId("vehicleA");
        req1.setContainerId("containerA");
        req1.setActionId("inspectionA");

        // Returns null before inspection is requested
        assertNull(inspectionActions.getInspectionStatus(req1.getActionId()));
        assertNull(inspectionActions.getCurrentInspection());

        // Current action and inspection status returns action after inspection is
        // requested
        inspectionActions.requestInspectionAction(req1);
        assertEquals(req1.getVehicleId(), inspectionActions.getCurrentInspection().getVehicleId());
        assertEquals(req1.getContainerId(), inspectionActions.getCurrentInspection().getContainerId());
        assertEquals(req1.getVehicleId(), inspectionActions.getInspectionStatus(req1.getActionId()).getVehicleId());
        assertEquals(req1.getContainerId(), inspectionActions.getInspectionStatus(req1.getActionId()).getContainerId());

        // Attempt to request new inspection while another inspection is in progress
        InspectionRequest req2 = new InspectionRequest();
        req2.setVehicleId("vehicleC");
        req2.setContainerId("containerC");
        req2.setActionId("inspectionC");
        inspectionActions.requestInspectionAction(req2);
        InspectionStatus status = inspectionActions.getCurrentInspection();

        // Current Action is still first request
        assertEquals(req1.getContainerId(), status.getContainerId());
        assertEquals(req1.getVehicleId(), status.getVehicleId());

        // New inspection request is in pending inspection list
        InspectionStatus pending = inspectionActions.getPendingInspections().getInspections().get(0);
        assertEquals(req2.getVehicleId(), pending.getVehicleId());
        assertEquals(req2.getContainerId(), pending.getContainerId());
    }

    /**
     * Test case to test {@link InspectionActions#completeInspection()} for
     * different possible parameters.
     */
    @Test
    public void completeInspectionTest() {
        // Populate Inspection actions with inspection requests
        InspectionRequest req1 = new InspectionRequest();
        req1.setVehicleId("vehicleA");
        req1.setContainerId("containerA");
        req1.setActionId("inspectionA");

        InspectionRequest req2 = new InspectionRequest();
        req2.setVehicleId("vehicleB");
        req2.setContainerId("containerB");
        req2.setActionId("inspectionB");

        InspectionRequest req3 = new InspectionRequest();
        req3.setVehicleId("vehicleC");
        req3.setContainerId("containerC");
        req3.setActionId("inspectionC");

        // Calling complete inspection with no current inspection does not add any
        // inspection to completed list
        inspectionActions.completeInspection();
        assertNull(inspectionActions.getCompletedInspections().getInspections());

        inspectionActions.requestInspectionAction(req1);
        inspectionActions.requestInspectionAction(req2);
        inspectionActions.requestInspectionAction(req3);

        // Completed Inspections is empty before completing any inspection but
        // inspection is in inspectionActions list of in progress inspections
        assertNull(inspectionActions.getCompletedInspections().getInspections());

        InspectionStatus req1Status = inspectionActions.getInspectionStatus(req1.getActionId());
        InspectionStatus req2Status = inspectionActions.getInspectionStatus(req2.getActionId());
        InspectionStatus req3Status = inspectionActions.getInspectionStatus(req3.getActionId());

        // First request becomes current action and additional requests are added to
        // pending inspections
        assertEquals(req1Status, inspectionActions.getCurrentInspection());
        assertTrue(inspectionActions.getPendingInspections().getInspections().contains(req2Status));
        assertTrue(inspectionActions.getPendingInspections().getInspections().contains(req3Status));

        // Complete inspection adds current inspection to completedInspections list and
        // sets next pending action as currentAction
        inspectionActions.completeInspection();

        assertTrue(inspectionActions.getCompletedInspections().getInspections().contains(req1Status));
        assertEquals(req2Status, inspectionActions.getCurrentInspection());
        assertTrue(inspectionActions.getPendingInspections().getInspections().contains(req3Status));
        assertNotNull(req1Status.getCompleted());
        assertEquals(InspectionStatus.StatusEnum.PASSED, req1Status.getStatus());

        // Completing inspection when no pending actions are left sets current action to
        // null
        // req2Status
        inspectionActions.completeInspection();

        // req3Status
        inspectionActions.completeInspection();

        assertNull(inspectionActions.getCurrentInspection());
        assertEquals(3, inspectionActions.getCompletedInspections().getInspections().size());

         // Get InspectionStatus will return most recent inspection even when it is completed
        // for a given vehicleID
        assertEquals(req3Status, inspectionActions.getInspectionStatus(req3.getActionId()));
    }

    /**
     * Test case to test {@link InspectionActions#requestHolding()} method and
     * holding vehicle interactions.
     */
    @Test
    public void requestHoldingTest() {
        // Populate Inspection actions with inspection requests
        InspectionRequest req1 = new InspectionRequest();
        req1.setVehicleId("vehicleA");
        req1.setContainerId("containerA");

        // Call proceed to holding with no current action
        assertNull(inspectionActions.getCurrentInspection());
        inspectionActions.proceedToHolding();

        // Call request holding with to current action
        inspectionActions.requestHolding();
        assertNull(inspectionActions.getCurrentInspection());

        // Request Inspection
        inspectionActions.requestInspectionAction(req1);
        InspectionStatus req1Status = inspectionActions.getCurrentInspection();

        assertEquals(StatusEnum.PENDING, req1Status.getStatus() );

        // Proceed To Holding
        inspectionActions.proceedToHolding();

        assertEquals(StatusEnum.PROCEED_TO_HOLDING, req1Status.getStatus() );

        // Request Holding
        inspectionActions.requestHolding();

        assertEquals(StatusEnum.HOLDING, req1Status.getStatus() );

        // Complete
        inspectionActions.completeInspection();
        assertEquals(StatusEnum.PASSED, req1Status.getStatus() );

    }
}
