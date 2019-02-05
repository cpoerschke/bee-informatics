import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import org.apache.solr.client.solrj.SolrQuery;
import org.apache.solr.client.solrj.SolrQuery.SortClause;
import org.apache.solr.client.solrj.impl.HttpSolrClient;
import org.apache.solr.client.solrj.impl.HttpSolrClient.Builder;
import org.apache.solr.client.solrj.request.QueryRequest;
import org.apache.solr.client.solrj.request.UpdateRequest;
import org.apache.solr.common.SolrDocument;
import org.apache.solr.common.SolrDocumentList;
import org.apache.solr.common.SolrInputDocument;
import org.apache.solr.common.util.NamedList;

public class CalcPosDelta {

  private static final String  READ_BASE_URL = "http://localhost:8983/solr";
  private static final String WRITE_BASE_URL = "http://localhost:8984/solr";

  private static final String  READ_COLLECTION_NAME = "bee-hive";
  private static final String WRITE_COLLECTION_NAME = "bee-steps";

  private static final String CAM_ID_FIELD_NAME = "cam_id_s";
  private static final String TIMESTAMP_FIELD_NAME = "timestamp_dt";

  public static void main(String[] args) throws Exception {

    if (args.length != 2) {
      System.out.println("Usage: <min bee id> <max bee id>");
      return;
    }

    final Integer minBeeId = Integer.valueOf(args[0]);
    final Integer maxBeeId = Integer.valueOf(args[1]);

    final HttpSolrClient readClient = new HttpSolrClient.Builder(READ_BASE_URL).build();
    final HttpSolrClient writeClient = new HttpSolrClient.Builder(WRITE_BASE_URL).build();

    for (int beeId = minBeeId; beeId <= maxBeeId; ++beeId) {
      processBeeData(beeId, readClient, writeClient);
    }
  }

  private static void processBeeData(int beeId, HttpSolrClient readClient, HttpSolrClient writeClient) throws Exception {

    final PosDeltasMap posDeltasMap = new PosDeltasMap();

    final SolrQuery solrQuery = new SolrQuery("bee_id_i:"+beeId);
    {
      final List<SortClause> sortClauses = new ArrayList<SortClause>(2);
      sortClauses.add(SortClause.asc(CAM_ID_FIELD_NAME));
      sortClauses.add(SortClause.asc(TIMESTAMP_FIELD_NAME));
      solrQuery.setSorts(sortClauses);
    }
    solrQuery.setRows(Integer.MAX_VALUE);

    final QueryRequest queryRequest = new QueryRequest(solrQuery);
    final NamedList<Object> queryResponse = readClient.request(queryRequest, READ_COLLECTION_NAME);

    final SolrDocumentList solrDocumentList = (SolrDocumentList)queryResponse.get("response");
    for (int ii=0; ii<solrDocumentList.size(); ++ii)
    {
      posDeltasMap.add(solrDocumentList, ii);
    }

    System.out.println("beeId="+beeId+" queryResponse.response.numFound="+solrDocumentList.getNumFound());
    if (solrDocumentList.isEmpty()) {
      return;
    }

    final UpdateRequest updateRequest = new UpdateRequest();
    {
      for (int ii=0; ii<solrDocumentList.size(); ++ii)
      {
        final Document currDocument = new Document(solrDocumentList.get(ii));
        final Double posDelta = posDeltasMap.lookup(currDocument);
        updateRequest.add(currDocument.toSolrInputDocument(posDelta), 1000 /* commitWithinMs */);
      }
    }
    final NamedList<Object> updateResponse = writeClient.request(updateRequest, WRITE_COLLECTION_NAME);
    System.out.println("updateResponse="+updateResponse);
  }

  private static class Document {

    private static final String POS_DELTA_FIELD_NAME = "pos_delta_f";

    private final SolrDocument solrDocument;

    public Document(SolrDocument solrDocument) {
      this.solrDocument = solrDocument;
    }

    public String camId() {
      return (String)solrDocument.getFieldValue(CAM_ID_FIELD_NAME);
    }

    public Date timestamp() {
      return (Date)solrDocument.getFieldValue(TIMESTAMP_FIELD_NAME);
    }

    public Integer xPos() {
      return (Integer)solrDocument.getFieldValue("x_pos_i");
    }

    public Integer yPos() {
      return (Integer)solrDocument.getFieldValue("y_pos_i");
    }

    public SolrInputDocument toSolrInputDocument(Double posDeltaValue) {
      final SolrInputDocument solrInputDocument = new SolrInputDocument();
      for (String field : solrDocument.keySet()) {
        if ("_version_".equals(field)) {
          continue; // because it's an internal Solr field
        }
        if (POS_DELTA_FIELD_NAME.equals(field)) {
          continue; // because we'll add it below
        }
        solrInputDocument.addField(field, solrDocument.get(field));
      }
      solrInputDocument.addField(POS_DELTA_FIELD_NAME, posDeltaValue);
      return solrInputDocument;
    }

  }

  private static class PosDeltasMap {

    private static final long TIMESTAMP_DELTA_MILLISECONDS_MAX = 340;

    final Map<String,List<Double>> keyToValsArrMap = new HashMap<String,List<Double>>();

    public PosDeltasMap() {
    }

    private static String mapKey(Document document) {
      return String.format(Locale.ROOT, "%s_%d", document.camId(), document.timestamp().getTime());
    }

    public void add(SolrDocumentList solrDocumentList, int ii) {
      final Document currDocument = new Document(solrDocumentList.get(ii));

      final String deltaMapKey = mapKey(currDocument);
      keyToValsArrMap.putIfAbsent(deltaMapKey, new ArrayList<Double>(1));

      for (int off=1; off<=ii; ++off)
      {
        final Document prevDocument = new Document(solrDocumentList.get(ii-off));

        if (!prevDocument.camId().equals(currDocument.camId())) {
          break;
        }

        if (prevDocument.timestamp().equals(currDocument.timestamp())) {
          continue;
        }

        if ((currDocument.timestamp().getTime() - prevDocument.timestamp().getTime()) <= TIMESTAMP_DELTA_MILLISECONDS_MAX) {

          final int deltaX = currDocument.xPos() - prevDocument.xPos();
          final int deltaY = currDocument.yPos() - prevDocument.yPos();

          final double delta = Math.sqrt( Math.pow(deltaX,2) + Math.pow(deltaY,2) );

          keyToValsArrMap.get(deltaMapKey).add(delta);
        } else {
          break;
        }
      }
    }

    public Double lookup(Document document) {
      final List<Double> posDeltas = keyToValsArrMap.get(mapKey(document));
      if (posDeltas.size() == 1) {
        return posDeltas.get(0); // one (potentially zero) value
      } else if (posDeltas.size() == 0) {
        return -1d; // no value (e.g. first sighting after return to the hive)
      } else {
        return -1d * posDeltas.size(); // multiple values
      }
    }

  }

}
