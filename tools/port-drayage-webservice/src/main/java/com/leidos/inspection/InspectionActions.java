package com.leidos.inspection;

import com.baeldung.openapi.api.InspectionApi;
import com.baeldung.openapi.model.InspectionRequest;
import com.baeldung.openapi.model.InspectionStatus;
import com.baeldung.openapi.model.InspectionStatus.StatusEnum;
import com.baeldung.openapi.model.InspectionStatusList;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

/**
 * {@link InspectionActions} is a spring bean for managing inspections. Only
 * allows for a single inspection to be in progress at a time and is stored as
 * the {@link InspectionActions#currentInspection}. Any inspections requested
 * while the current inspection is not yet completed will be added to
 * {@link InspectionActions#pendingInspections} list. This list is then used to
 * populate the current action after an action is completed. Each completed
 * action is stored in the {@link InspectionActions#completedInspections} list.
 * 
 * @author Paul Bourelly
 */
@Component
public class InspectionActions  {

    private static Logger logger = LoggerFactory.getLogger(InspectionActions.class);

    // List of pending inspection statuses
    private InspectionStatusList pendingInspections = new InspectionStatusList();

    // List of completed inspections
    private InspectionStatusList completedInspections = new InspectionStatusList();

    // Current in progress action
    private InspectionStatus currentInspection;

    /**
     * Empty Constructor
     */
    public InspectionActions() {
    }

    /**
     * Getter for pending inspection.
     * 
     * @return {@link InspectionStatusList} list of pending inspection.
     */
    public InspectionStatusList getPendingInspections() {
        return pendingInspections;
    }

    /**
     * Getter for completed inspections.
     * 
     * @return {@link InspectionStatusList} list of completed inspection.
     */
    public InspectionStatusList getCompletedInspections() {
        return completedInspections;
    }

    /**
     * Getter for current action.
     * 
     * @return {@link InspectionStatus} current action.
     */
    public InspectionStatus getCurrentInspection() {
        return currentInspection;
    }

    /**
     * Create {@link InspectionStatus} for valid {@link InspectionRequest}s. Valid
     * requests require vehicleId and containerId. Set as current inspection if none
     * already exists or added to list of pending inspections if current inspection
     * is still in progress.
     * 
     * @param request {@link InspectionRequest} Request to load container.
     */
    public void requestInspectionAction(InspectionRequest request) {
        if (request != null) {
            InspectionStatus inspectionStatus = new InspectionStatus();
            inspectionStatus.setContainerId(request.getContainerId());
            inspectionStatus.setVehicleId(request.getVehicleId());
            inspectionStatus.setActionId(request.getActionId());
            inspectionStatus.setStatus(StatusEnum.PENDING);
            inspectionStatus.setRequested(System.currentTimeMillis());

            // If no current action set as current action
            if (currentInspection == null) {
                currentInspection = inspectionStatus;
            }

            // else add to list of pending action
            else {
                pendingInspections.addInspectionsItem(inspectionStatus);
            }
        } else {
            logger.warn("Attempted to add null InspectionRequest!");
        }

    }

    /**
     * Searches all inspections ( pending, current and completed ) for a given
     * actionId and returns {@link InspectionStatus}. Returns null if non is found.
     * 
     * @param actionId unique string to identify action
     * @return {@link InspectionStatus} for given action. Null if no action is found 
     * or null action id is provided.
     */
    public InspectionStatus getInspectionStatus(String actionId) {
        if (actionId != null) {
            // Pending actions ( null check since arraylist is initially null )
            if (pendingInspections.getInspections() != null) {
                for (InspectionStatus inspectionStatus : pendingInspections.getInspections()) {
                    if (actionId.equals(inspectionStatus.getActionId())) {
                        return inspectionStatus;
                    }
                }
            }
            // Current action
            if (currentInspection != null && actionId.equals(currentInspection.getActionId())) {
                return currentInspection;
            }
            // Completed actions ( null check since arraylist is initially null )
            if (completedInspections.getInspections() != null) {
                for (InspectionStatus inspectionStatus : completedInspections.getInspections()) {
                    if (actionId.equals(inspectionStatus.getActionId())) {
                        return inspectionStatus;
                    }
                }
            }
            logger.warn(String.format("No inspection action with action ID %s !", actionId));
            return null;
        }
        logger.warn("Null action id is not valid!");
        return null;
    }

    /**
     * To complete inspection set {@link InspectionActions#currentInspection} status
     * to {@link StatusEnum#PASSED}. Current inspection is then added to the
     * completed inspections list and the first pending action , if any are present
     * is set as the new current action. If no pending actions are present the
     * current action is set to null.
     * 
     */
    public void completeInspection() {
        if (currentInspection != null) {
            // Set current inspection to PASSED
            currentInspection.setStatus(StatusEnum.PASSED);
            currentInspection.setCompleted(System.currentTimeMillis());
            completedInspections.addInspectionsItem(currentInspection);
            // If there are any pending actions set them to current
            if (pendingInspections.getInspections() != null && !pendingInspections.getInspections().isEmpty()) {
                currentInspection = pendingInspections.getInspections().remove(0);
            }
            // else set current to null
            else {
                currentInspection = null;
            }

        } else
            logger.warn("No current inspection in progress!");
    }

    /**
     * To indicate that the vehicle under inspection is required to proceed to the
     * holding area for further inspection an operator will use this request to set
     * {@link InspectionActions#currentInspection} status to.
     * {@link StatusEnum#PROCEED_TO_HOLDING}.
     */
    public void proceedToHolding() {
        if (currentInspection != null) {
            currentInspection.setStatus(StatusEnum.PROCEED_TO_HOLDING);
        } else
            logger.warn("No current inspection in progress!");
    }

    /**
     * Vehicle under current inspection, which has been instructed to proceed to
     * holding area by operator will indicate it has arrived at the holding area and
     * is prepared for further inspection with this request which sets the status of
     * the {@link InspectionActions#currentInspection} to
     * {@link StatusEnum#HOLDING}.
     */
    public void requestHolding() {
        if (currentInspection != null) {
            currentInspection.setStatus(StatusEnum.HOLDING);
        } else
            logger.warn("No current inspection in progress!");
    }

    /**
     * Clear all inspection actions
     */
    public void clear() {
        if ( pendingInspections.getInspections() != null )
            pendingInspections.getInspections().clear();
        if ( completedInspections.getInspections() != null )
            completedInspections.getInspections().clear();
        currentInspection = null;
    }

}
