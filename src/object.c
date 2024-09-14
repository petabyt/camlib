// Camlib Object service
// This is mainly for file tables
#include <stdlib.h>
#include <camlib.h>
#include <string.h>

struct ObjectCache {
	// List of indexes to priority objects
	// List of indexes to access object list -> status[i].info or status[i]->thumb
	// GetObjects will traverse index list and return downloaded objects

	// Unoptimized array
	struct ObjectStatus {
		int handle;
		int is_downloaded;
		int is_priority;
		struct PtpObjectInfo info;
	}**status;
	int num_downloaded;
	int status_length;
	int curr;
	ptp_object_found_callback *callback;
	void *arg;
};

void ptp_object_service_sort(struct PtpRuntime *r, struct ObjectCache *oc) {
	ptp_mutex_lock(r);

	// Sort an index buffer?

	ptp_mutex_unlock(r);
}

void ptp_object_service_add_priority(struct PtpRuntime *r, struct ObjectCache *oc, int handle) {
	ptp_mutex_lock(r);

	for (int i = 0; i < oc->status_length; i++) {
		if (oc->status[i]->handle == handle) {
			oc->status[i]->is_priority = 1;
			break;
		}
	}

	ptp_mutex_unlock(r);
}

int ptp_object_service_step(struct PtpRuntime *r, struct ObjectCache *oc) {
	ptp_mutex_lock(r);

	if (oc->curr >= oc->status_length) abort();
	if (oc->curr == oc->status_length - 1) {
		ptp_mutex_unlock(r);
		return 0; // response code DONE
	}

	int curr = oc->curr;
	for (int i = 0; i < oc->status_length; i++) {
		if (oc->status[i]->is_priority) {
			curr = i;
			break;
		}
	}

	int handle = oc->status[curr]->handle;

	int rc = ptp_get_object_info(r, handle, &oc->status[curr]->info);
	if (rc == PTP_CHECK_CODE) {
		oc->status[curr]->is_downloaded = 0;
		oc->curr++;
		ptp_mutex_unlock(r);
		return 0;
	}
	if (rc) {
		ptp_mutex_unlock(r);
		return rc;
	}

	oc->status[curr]->is_downloaded = 1;
	oc->num_downloaded++;

	oc->callback(r, &oc->status[curr]->info, oc->arg);

	oc->curr++;

	ptp_mutex_unlock(r);

	return 0;
}

int ptp_object_service_length(struct PtpRuntime *r, struct ObjectCache *oc) {
	return oc->num_downloaded;
}

struct PtpObjectInfo *ptp_object_service_get_index(struct PtpRuntime *r, struct ObjectCache *oc, int req_i) {
	int count = 0;
	ptp_mutex_lock(r);
	for (int i = 0; i < oc->status_length; i++) {
		if (oc->status[i]->is_downloaded) {
			if (req_i == count) {
				ptp_mutex_unlock(r);
				return &oc->status[i]->info;
			}
			count++;
		}
	}

	ptp_mutex_unlock(r);
	return NULL;
}

struct PtpObjectInfo *ptp_object_service_get(struct PtpRuntime *r, struct ObjectCache *oc, int handle) {
	ptp_mutex_lock(r);
	for (int i = 0; i < oc->status_length; i++) {
		if (oc->status[i]->handle == handle) {
			if (oc->status[i]->is_downloaded) {
				ptp_mutex_unlock(r);
				return &oc->status[i]->info;
			} else {
				ptp_mutex_unlock(r);
				return NULL;
			}
		}
	}
	ptp_mutex_unlock(r);
	return NULL;
}

struct ObjectCache *ptp_create_object_service(int *handles, int length, ptp_object_found_callback *callback, void *arg) {
	struct ObjectCache *oc = malloc(sizeof(struct ObjectCache));
	oc->callback = callback;
	oc->status_length = length;
	oc->curr = 0;
	oc->num_downloaded = 0;
	oc->status = malloc(sizeof(struct ObjectStatus *) * length);
	for (int i = 0; i < length; i++) {
		struct ObjectStatus *os = (struct ObjectStatus *)malloc(sizeof(struct ObjectStatus));
		os->handle = handles[i];
		os->is_downloaded = 0;
		os->is_priority = 0;
		oc->status[i] = os;
	}

	return oc;
}

void ptp_free_object_service(struct ObjectCache *oc) {
	free(oc->status);
	free(oc);
}
